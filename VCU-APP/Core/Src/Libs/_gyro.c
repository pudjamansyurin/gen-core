/*
 * mems.c
 *
 *  Created on: Aug 23, 2019
 *      Author: Puja
 */
/* Includes ------------------------------------------------------------------*/
#include "Libs/_gyro.h"
#include "Drivers/_mpu6050.h"
#include "Nodes/VCU.h"
#include "i2c.h"

/* External variables ---------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c3;
extern vcu_t VCU;
extern void MX_I2C3_Init(void);

/* Private variables ----------------------------------------------------------*/
static MPU6050 mpu;
static mems_t mems;
//static mems_t calibrator;
static mems_decision_t decider;
static motion_t motion = {0};

/* Public functions implementation --------------------------------------------*/
void GYRO_Init(void) {
    MPU6050_Result result;

    do {
        LOG_StrLn("Gyro:Init");

        MX_I2C3_DeInit();
        MX_I2C3_Init();

        // MOSFET Control
        HAL_GPIO_WritePin(INT_GYRO_PWR_GPIO_Port, INT_GYRO_PWR_Pin, 0);
        _DelayMS(2000);
        HAL_GPIO_WritePin(INT_GYRO_PWR_GPIO_Port, INT_GYRO_PWR_Pin, 1);
        _DelayMS(500);

        // module initialization
        result = MPU6050_Init(&hi2c3, &mpu, MPU6050_Device_0, MPU6050_Accelerometer_16G, MPU6050_Gyroscope_2000s);

    } while (result != MPU6050_Result_Ok);

    // Set calibrator
    //    calibrator = GYRO_Average(500);
    //    LOG_StrLn("Gyro:Calibrated");
}

mems_t GYRO_Average(uint16_t sample) {
    static uint8_t calibrated = 0;
    int32_t aX=0, aY=0, aZ=0;
    int32_t gX=0, gY=0, gZ=0;
    double sqrtRoll, sqrtPitch, sqrtYaw;
    double tmp=0;
    MPU6050_Result status;

    // sampling
    for (uint16_t i = 0; i < sample; i++) {
        // read sensor
        do {
            status = MPU6050_ReadAll(&hi2c3, &mpu);

            if(status != MPU6050_Result_Ok) {
                GYRO_Init();
            }
        } while(status != MPU6050_Result_Ok);

        // sum all value
        // convert the RAW values into dps (deg/s)
        aX += mpu.Gyroscope_X / mpu.Gyro_Mult;
        aY += mpu.Gyroscope_Y / mpu.Gyro_Mult;
        aZ += mpu.Gyroscope_Z / mpu.Gyro_Mult;
        // convert the RAW values into acceleration in 'g'
        gX += mpu.Accelerometer_X / mpu.Acce_Mult;
        gY += mpu.Accelerometer_Y / mpu.Acce_Mult;
        gZ += mpu.Accelerometer_Z / mpu.Acce_Mult;
        tmp += mpu.Temperature;
    }

    // calculate the average
    aX /= sample;
    aY /= sample;
    aZ /= sample;
    gX /= sample;
    gY /= sample;
    gZ /= sample;
    tmp /= sample;

    // set for calibration
    if (calibrated) {
        //        aX -= calibrator.accelerometer.x;
        //        aY -= calibrator.accelerometer.y;
        //        aZ -= calibrator.accelerometer.z;
        //        gX -= calibrator.gyroscope.x;
        //        gY -= calibrator.gyroscope.y;
        //        gZ -= calibrator.gyroscope.z;
        //        tmp -= calibrator.temperature;

        // Calculating Roll and Pitch from the accelerometer data
        sqrtYaw = sqrt(pow(gX, 2) + pow(gY, 2));
        sqrtRoll = sqrt(pow(gX, 2) + pow(gZ, 2));
        sqrtPitch = sqrt(pow(gY, 2) + pow(gZ, 2));

        motion.yaw = sqrtYaw == 0 ? 0 : RAD2DEG(atan(gZ / sqrtYaw));
        motion.roll = sqrtRoll == 0 ? 0 : RAD2DEG(atan(gY / sqrtRoll));
        motion.pitch = sqrtPitch == 0 ? 0 : RAD2DEG(atan(gX / sqrtPitch));
    } else {
        calibrated = 1;
    }

    // save the result
    mems.accelerometer.x = aX;
    mems.accelerometer.y = aY;
    mems.accelerometer.z = aZ;
    mems.gyroscope.x = gX;
    mems.gyroscope.y = gY;
    mems.gyroscope.z = gZ;
    mems.temperature = tmp;

    return mems;
}

mems_decision_t GYRO_Decision(uint16_t sample) {
    static uint16_t g_max = 0;

    // get mems data
    mems = GYRO_Average(sample);

    // calculate g-force
    decider.crash.value = sqrt(
            pow(mems.accelerometer.x, 2) +
            pow(mems.accelerometer.y, 2) +
            pow(mems.accelerometer.z, 2)
    ) / GRAVITY_FORCE;
    decider.crash.state = (decider.crash.value > ACCELEROMETER_LIMIT);

    // capture max g-force
    if(decider.crash.value > g_max) {
        g_max = decider.crash.value;
    }

    // calculate movement change
    decider.fall.value = sqrt(pow(abs(motion.roll),2) + pow(abs(motion.pitch),2));
    decider.fall.state = decider.fall.value > GYROSCOPE_LIMIT || motion.yaw < GYROSCOPE_LIMIT;

    // record as report
    VCU.d.motion.pitch = motion.pitch;
    VCU.d.motion.roll = motion.roll;

    // debugger
    //	Gyro_RawDebugger();

    return decider;
}

void Gyro_Debugger(mems_decision_t *decider) {
    // calculated data
    LOG_Str("IMU:Accel[");
    LOG_Int(decider->crash.value * 100 / ACCELEROMETER_LIMIT);
    LOG_Str(" %] = ");
    LOG_Int(decider->crash.value);
    LOG_Str(" / ");
    LOG_Int(ACCELEROMETER_LIMIT);
    LOG_Enter();
    LOG_Str("IMU:Gyros[");
    LOG_Int(decider->fall.value * 100 / GYROSCOPE_LIMIT);
    LOG_Str(" %] = ");
    LOG_Int(decider->fall.value);
    LOG_Str(" / ");
    LOG_Int(GYROSCOPE_LIMIT);
    LOG_Enter();
}

void Gyro_RawDebugger(void) {
    // raw data
    char str[100];
    sprintf(str,
            "Accelerometer\n- X:%d\n- Y:%d\n- Z:%d\n"
            "Gyroscope\n- X:%d\n- Y:%d\n- Z:%d\n"
            "Temperature: %d\n\n",
            mems.accelerometer.x, mems.accelerometer.y, mems.accelerometer.z,
            mems.gyroscope.x, mems.gyroscope.y, mems.gyroscope.z,
            (int8_t) mems.temperature
    );
    LOG_Str(str);
}

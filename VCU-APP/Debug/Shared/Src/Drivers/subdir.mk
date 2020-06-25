################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/geni/STM32CubeIDE/workspace_1.3.0/VCU/Shared/Src/Drivers/_at.c \
/home/geni/STM32CubeIDE/workspace_1.3.0/VCU/Shared/Src/Drivers/_canbus.c \
/home/geni/STM32CubeIDE/workspace_1.3.0/VCU/Shared/Src/Drivers/_crc.c \
/home/geni/STM32CubeIDE/workspace_1.3.0/VCU/Shared/Src/Drivers/_ee24xx.c \
/home/geni/STM32CubeIDE/workspace_1.3.0/VCU/Shared/Src/Drivers/_log.c 

OBJS += \
./Shared/Src/Drivers/_at.o \
./Shared/Src/Drivers/_canbus.o \
./Shared/Src/Drivers/_crc.o \
./Shared/Src/Drivers/_ee24xx.o \
./Shared/Src/Drivers/_log.o 

C_DEPS += \
./Shared/Src/Drivers/_at.d \
./Shared/Src/Drivers/_canbus.d \
./Shared/Src/Drivers/_crc.d \
./Shared/Src/Drivers/_ee24xx.d \
./Shared/Src/Drivers/_log.d 


# Each subdirectory must supply rules for building sources it contributes
Shared/Src/Drivers/_at.o: /home/geni/STM32CubeIDE/workspace_1.3.0/VCU/Shared/Src/Drivers/_at.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F423xx -DDEBUG -c -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I"/home/geni/STM32CubeIDE/workspace_1.3.0/VCU/Shared/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Shared/Src/Drivers/_at.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Shared/Src/Drivers/_canbus.o: /home/geni/STM32CubeIDE/workspace_1.3.0/VCU/Shared/Src/Drivers/_canbus.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F423xx -DDEBUG -c -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I"/home/geni/STM32CubeIDE/workspace_1.3.0/VCU/Shared/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Shared/Src/Drivers/_canbus.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Shared/Src/Drivers/_crc.o: /home/geni/STM32CubeIDE/workspace_1.3.0/VCU/Shared/Src/Drivers/_crc.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F423xx -DDEBUG -c -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I"/home/geni/STM32CubeIDE/workspace_1.3.0/VCU/Shared/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Shared/Src/Drivers/_crc.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Shared/Src/Drivers/_ee24xx.o: /home/geni/STM32CubeIDE/workspace_1.3.0/VCU/Shared/Src/Drivers/_ee24xx.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F423xx -DDEBUG -c -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I"/home/geni/STM32CubeIDE/workspace_1.3.0/VCU/Shared/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Shared/Src/Drivers/_ee24xx.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Shared/Src/Drivers/_log.o: /home/geni/STM32CubeIDE/workspace_1.3.0/VCU/Shared/Src/Drivers/_log.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F423xx -DDEBUG -c -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I"/home/geni/STM32CubeIDE/workspace_1.3.0/VCU/Shared/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Shared/Src/Drivers/_log.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"


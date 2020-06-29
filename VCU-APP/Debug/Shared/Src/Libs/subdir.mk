################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/geni/STM32CubeIDE/workspace_1.3.0/gen-core/Shared/Src/Libs/_eeprom.c \
/home/geni/STM32CubeIDE/workspace_1.3.0/gen-core/Shared/Src/Libs/_simcom.c \
/home/geni/STM32CubeIDE/workspace_1.3.0/gen-core/Shared/Src/Libs/_utils.c 

OBJS += \
./Shared/Src/Libs/_eeprom.o \
./Shared/Src/Libs/_simcom.o \
./Shared/Src/Libs/_utils.o 

C_DEPS += \
./Shared/Src/Libs/_eeprom.d \
./Shared/Src/Libs/_simcom.d \
./Shared/Src/Libs/_utils.d 


# Each subdirectory must supply rules for building sources it contributes
Shared/Src/Libs/_eeprom.o: /home/geni/STM32CubeIDE/workspace_1.3.0/gen-core/Shared/Src/Libs/_eeprom.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F423xx -DDEBUG -c -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I"/home/geni/STM32CubeIDE/workspace_1.3.0/gen-core/Shared/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Shared/Src/Libs/_eeprom.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Shared/Src/Libs/_simcom.o: /home/geni/STM32CubeIDE/workspace_1.3.0/gen-core/Shared/Src/Libs/_simcom.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F423xx -DDEBUG -c -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I"/home/geni/STM32CubeIDE/workspace_1.3.0/gen-core/Shared/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Shared/Src/Libs/_simcom.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Shared/Src/Libs/_utils.o: /home/geni/STM32CubeIDE/workspace_1.3.0/gen-core/Shared/Src/Libs/_utils.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F423xx -DDEBUG -c -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I"/home/geni/STM32CubeIDE/workspace_1.3.0/gen-core/Shared/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Shared/Src/Libs/_utils.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"


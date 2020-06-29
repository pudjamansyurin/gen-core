################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/geni/STM32CubeIDE/workspace_1.3.0/gen-core/Shared/Src/DMA/_dma_simcom.c 

OBJS += \
./Shared/Src/DMA/_dma_simcom.o 

C_DEPS += \
./Shared/Src/DMA/_dma_simcom.d 


# Each subdirectory must supply rules for building sources it contributes
Shared/Src/DMA/_dma_simcom.o: /home/geni/STM32CubeIDE/workspace_1.3.0/gen-core/Shared/Src/DMA/_dma_simcom.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F423xx -DDEBUG -c -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I"/home/geni/STM32CubeIDE/workspace_1.3.0/gen-core/Shared/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Shared/Src/DMA/_dma_simcom.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"


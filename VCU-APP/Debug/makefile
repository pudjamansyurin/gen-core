################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Shared/Src/Libs/subdir.mk
-include Shared/Src/Drivers/subdir.mk
-include Shared/Src/DMA/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/subdir.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/Nodes/subdir.mk
-include Core/Src/Libs/subdir.mk
-include Core/Src/Drivers/subdir.mk
-include Core/Src/DMA/subdir.mk
-include Core/Src/Assets/subdir.mk
-include Core/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
VCU-APP.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
VCU-APP.list \

OBJCOPY_HEX += \
VCU-APP.hex \

OBJCOPY_BIN += \
VCU-APP.bin \


# All Target
all: VCU-APP.elf secondary-outputs

# Tool invocations
VCU-APP.elf: $(OBJS) $(USER_OBJS) /home/geni/STM32CubeIDE/workspace_1.3.0/VCU/VCU-APP/STM32F423VHTX_FLASH.ld
	arm-none-eabi-gcc -o "VCU-APP.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"/home/geni/STM32CubeIDE/workspace_1.3.0/VCU/VCU-APP/STM32F423VHTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="VCU-APP.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

VCU-APP.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "VCU-APP.list"
	@echo 'Finished building: $@'
	@echo ' '

VCU-APP.hex: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "VCU-APP.hex"
	@echo 'Finished building: $@'
	@echo ' '

VCU-APP.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "VCU-APP.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

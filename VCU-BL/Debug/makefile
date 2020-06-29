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
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/Libs/subdir.mk
-include Core/Src/Drivers/subdir.mk
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
VCU-BL.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
VCU-BL.list \

OBJCOPY_BIN += \
VCU-BL.bin \


# All Target
all: VCU-BL.elf secondary-outputs

# Tool invocations
VCU-BL.elf: $(OBJS) $(USER_OBJS) /home/geni/STM32CubeIDE/workspace_1.3.0/gen-core/VCU-BL/STM32F423VHTX_FLASH.ld
	arm-none-eabi-gcc -o "VCU-BL.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"/home/geni/STM32CubeIDE/workspace_1.3.0/gen-core/VCU-BL/STM32F423VHTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="VCU-BL.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

VCU-BL.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "VCU-BL.list"
	@echo 'Finished building: $@'
	@echo ' '

VCU-BL.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "VCU-BL.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

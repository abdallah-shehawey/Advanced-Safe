################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include MCAL_Layer/USART/subdir.mk
-include MCAL_Layer/EEPROM/subdir.mk
-include MCAL_Layer/DIO/subdir.mk
-include HAL_Layer/CLCD/subdir.mk
-include APP_Layer/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := Advanced_Safe
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
Advanced_Safe.lss \

FLASH_IMAGE += \
Advanced_Safe.hex \

SIZEDUMMY += \
sizedummy \


# All Target
all: main-build

# Main-build Target
main-build: Advanced_Safe.elf secondary-outputs

# Tool invocations
Advanced_Safe.elf: $(OBJS) $(USER_OBJS) makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,Advanced_Safe.map -mmcu=atmega32 -o "Advanced_Safe.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

Advanced_Safe.lss: Advanced_Safe.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S Advanced_Safe.elf  >"Advanced_Safe.lss"
	@echo 'Finished building: $@'
	@echo ' '

Advanced_Safe.hex: Advanced_Safe.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex Advanced_Safe.elf  "Advanced_Safe.hex"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: Advanced_Safe.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega32 Advanced_Safe.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) Advanced_Safe.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents main-build

-include ../makefile.targets

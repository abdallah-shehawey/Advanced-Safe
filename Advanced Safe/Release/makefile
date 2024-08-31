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


BUILD_ARTIFACT_NAME := The_Safe
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
The_Safe.lss \

FLASH_IMAGE += \
The_Safe.hex \

EEPROM_IMAGE += \
The_Safe.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: main-build

# Main-build Target
main-build: The_Safe.elf secondary-outputs

# Tool invocations
The_Safe.elf: $(OBJS) $(USER_OBJS) makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,The_Safe.map -mmcu=atmega32 -o "The_Safe.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

The_Safe.lss: The_Safe.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S The_Safe.elf  >"The_Safe.lss"
	@echo 'Finished building: $@'
	@echo ' '

The_Safe.hex: The_Safe.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex The_Safe.elf  "The_Safe.hex"
	@echo 'Finished building: $@'
	@echo ' '

The_Safe.eep: The_Safe.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex The_Safe.elf  "The_Safe.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: The_Safe.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega32 The_Safe.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(EEPROM_IMAGE)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) The_Safe.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents main-build

-include ../makefile.targets

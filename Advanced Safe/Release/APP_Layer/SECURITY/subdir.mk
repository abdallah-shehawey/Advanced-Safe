################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../APP_Layer/SECURITY/SECURITY_prog.c 

OBJS += \
./APP_Layer/SECURITY/SECURITY_prog.o 

C_DEPS += \
./APP_Layer/SECURITY/SECURITY_prog.d 


# Each subdirectory must supply rules for building sources it contributes
APP_Layer/SECURITY/%.o: ../APP_Layer/SECURITY/%.c APP_Layer/SECURITY/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega32 -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



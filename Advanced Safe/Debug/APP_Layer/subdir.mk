################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../APP_Layer/Advanced_Safe.c 

OBJS += \
./APP_Layer/Advanced_Safe.o 

C_DEPS += \
./APP_Layer/Advanced_Safe.d 


# Each subdirectory must supply rules for building sources it contributes
APP_Layer/%.o: ../APP_Layer/%.c APP_Layer/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -O0 -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega32 -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



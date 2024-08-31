################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../APP_Layer/Advanced_Safe.c \
../APP_Layer/Security.c 

OBJS += \
./APP_Layer/Advanced_Safe.o \
./APP_Layer/Security.o 

C_DEPS += \
./APP_Layer/Advanced_Safe.d \
./APP_Layer/Security.d 


# Each subdirectory must supply rules for building sources it contributes
APP_Layer/%.o: ../APP_Layer/%.c APP_Layer/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega32 -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



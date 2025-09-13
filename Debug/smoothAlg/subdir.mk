################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../smoothAlg/smoothAlg.c 

OBJS += \
./smoothAlg/smoothAlg.o 

C_DEPS += \
./smoothAlg/smoothAlg.d 


# Each subdirectory must supply rules for building sources it contributes
smoothAlg/%.o: ../smoothAlg/%.c smoothAlg/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -v -mmcu=atmega32 -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



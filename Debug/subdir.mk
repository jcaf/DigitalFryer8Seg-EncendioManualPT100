################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../disp7s_applevel.c \
../display7s_setup.c \
../main.c \
../psmode_operative.c \
../psmode_program.c \
../psmode_viewTemp.c 

OBJS += \
./disp7s_applevel.o \
./display7s_setup.o \
./main.o \
./psmode_operative.o \
./psmode_program.o \
./psmode_viewTemp.o 

C_DEPS += \
./disp7s_applevel.d \
./display7s_setup.d \
./main.d \
./psmode_operative.d \
./psmode_program.d \
./psmode_viewTemp.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -v -mmcu=atmega32 -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



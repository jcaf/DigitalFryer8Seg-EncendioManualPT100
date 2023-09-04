################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include utils/subdir.mk
-include timing/subdir.mk
-include smoothAlg/subdir.mk
-include pinGetLevel/subdir.mk
-include pid/subdir.mk
-include indicator/subdir.mk
-include ikb/subdir.mk
-include groupdig/subdir.mk
-include blink/subdir.mk
-include adc/subdir.mk
-include Temperature/subdir.mk
-include SPI/subdir.mk
-include MAX6675/subdir.mk
-include Display7S/subdir.mk
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

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
DigitalFryerDisplay8Seg-EncendidoManual.lss \

FLASH_IMAGE += \
DigitalFryerDisplay8Seg-EncendidoManual.hex \

SIZEDUMMY += \
sizedummy \


# All Target
all: DigitalFryerDisplay8Seg-EncendidoManual.elf secondary-outputs

# Tool invocations
DigitalFryerDisplay8Seg-EncendidoManual.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,DigitalFryerDisplay8Seg-EncendidoManual.map -mmcu=atmega32 -o "DigitalFryerDisplay8Seg-EncendidoManual.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

DigitalFryerDisplay8Seg-EncendidoManual.lss: DigitalFryerDisplay8Seg-EncendidoManual.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S DigitalFryerDisplay8Seg-EncendidoManual.elf  >"DigitalFryerDisplay8Seg-EncendidoManual.lss"
	@echo 'Finished building: $@'
	@echo ' '

DigitalFryerDisplay8Seg-EncendidoManual.hex: DigitalFryerDisplay8Seg-EncendidoManual.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex DigitalFryerDisplay8Seg-EncendidoManual.elf  "DigitalFryerDisplay8Seg-EncendidoManual.hex"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: DigitalFryerDisplay8Seg-EncendidoManual.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega32 DigitalFryerDisplay8Seg-EncendidoManual.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) DigitalFryerDisplay8Seg-EncendidoManual.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents

-include ../makefile.targets

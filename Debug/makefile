################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
OffChipMqtt_ATMEGA32U2.lss \

FLASH_IMAGE += \
OffChipMqtt_ATMEGA32U2.hex \

EEPROM_IMAGE += \
OffChipMqtt_ATMEGA32U2.eep \

SIZEDUMMY += \
sizedummy \

AVRDUDEDUMMY += \
avrdudedummy \


# All Target
all: OffChipMqtt_ATMEGA32U2.elf secondary-outputs

# Tool invocations
OffChipMqtt_ATMEGA32U2.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C++ Linker'
	avr-g++ -Wl,-Map,OffChipMqtt_ATMEGA32U2.map,--cref -mrelax -Wl,--gc-sections -L"C:\siriusxm\data\workspaces\playing\AtmelLibrary\Debug" -L"C:\siriusxm\data\workspaces\playing\USBLibrary\Debug" -mmcu=atmega32u2 -o "OffChipMqtt_ATMEGA32U2.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

OffChipMqtt_ATMEGA32U2.lss: OffChipMqtt_ATMEGA32U2.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S OffChipMqtt_ATMEGA32U2.elf  >"OffChipMqtt_ATMEGA32U2.lss"
	@echo 'Finished building: $@'
	@echo ' '

OffChipMqtt_ATMEGA32U2.hex: OffChipMqtt_ATMEGA32U2.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex OffChipMqtt_ATMEGA32U2.elf  "OffChipMqtt_ATMEGA32U2.hex"
	@echo 'Finished building: $@'
	@echo ' '

OffChipMqtt_ATMEGA32U2.eep: OffChipMqtt_ATMEGA32U2.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex OffChipMqtt_ATMEGA32U2.elf  "OffChipMqtt_ATMEGA32U2.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: OffChipMqtt_ATMEGA32U2.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega32u2 OffChipMqtt_ATMEGA32U2.elf
	@echo 'Finished building: $@'
	@echo ' '

avrdudedummy: OffChipMqtt_ATMEGA32U2.elf
	@echo 'Invoking: AVRDude'
	avrdude -pm32u2 -carduino -PCOM11 -b19200 -V -Uflash:w:OffChipMqtt_ATMEGA32U2.hex:a -Ueeprom:w:OffChipMqtt_ATMEGA32U2.eep:a
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EEPROM_IMAGE)$(ELFS)$(FLASH_IMAGE)$(LSS)$(S_DEPS)$(CXX_DEPS)$(S_UPPER_DEPS)$(C++_DEPS)$(ASM_DEPS)$(CC_DEPS)$(AVRDUDEDUMMY)$(CPP_DEPS)$(C_UPPER_DEPS)$(SIZEDUMMY) OffChipMqtt_ATMEGA32U2.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY) $(AVRDUDEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

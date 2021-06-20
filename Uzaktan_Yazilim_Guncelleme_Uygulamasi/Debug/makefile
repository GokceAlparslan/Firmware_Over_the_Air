################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
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
Uzaktan_Yazilim_Guncelleme_Uygulamasi.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
Uzaktan_Yazilim_Guncelleme_Uygulamasi.list \

OBJCOPY_HEX += \
Uzaktan_Yazilim_Guncelleme_Uygulamasi.hex \

OBJCOPY_BIN += \
Uzaktan_Yazilim_Guncelleme_Uygulamasi.bin \


# All Target
all: Uzaktan_Yazilim_Guncelleme_Uygulamasi.elf secondary-outputs

# Tool invocations
Uzaktan_Yazilim_Guncelleme_Uygulamasi.elf: $(OBJS) $(USER_OBJS) C:\Workspace_STM32CubeIde\Uzaktan_Yazilim_Guncelleme_Uygulamasi\STM32F407VGTX_FLASH.ld
	arm-none-eabi-gcc -o "Uzaktan_Yazilim_Guncelleme_Uygulamasi.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Workspace_STM32CubeIde\Uzaktan_Yazilim_Guncelleme_Uygulamasi\STM32F407VGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="Uzaktan_Yazilim_Guncelleme_Uygulamasi.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

Uzaktan_Yazilim_Guncelleme_Uygulamasi.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "Uzaktan_Yazilim_Guncelleme_Uygulamasi.list"
	@echo 'Finished building: $@'
	@echo ' '

Uzaktan_Yazilim_Guncelleme_Uygulamasi.hex: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "Uzaktan_Yazilim_Guncelleme_Uygulamasi.hex"
	@echo 'Finished building: $@'
	@echo ' '

Uzaktan_Yazilim_Guncelleme_Uygulamasi.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "Uzaktan_Yazilim_Guncelleme_Uygulamasi.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccstheia151/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS

GEN_OPTS__FLAG := @"syscfg/device.opt" 
GEN_CMDS__FLAG := -Wl,-l"syscfg/device_linker.cmd" 

ORDERED_OBJS += \
"./empty.o" \
"./syscfg/ti_msp_dl_config.o" \
"./startup_mspm0g350x_ticlang.o" \
"./BSP/Buzzer/buzzer.o" \
"./BSP/Eight_Tracking/app_irtracking.o" \
"./BSP/Filter.o" \
"./BSP/delay.o" \
"./BSP/usart.o" \
"./BSP/Key/key.o" \
"./BSP/LED/led.o" \
"./BSP/MPU6050/app_mpu6050.o" \
"./BSP/MPU6050/bsp_mpu6050.o" \
"./BSP/Motor/app_motor.o" \
"./BSP/Motor/app_motor_usart.o" \
"./BSP/Motor/bsp_motor_usart.o" \
"./BSP/Questions/questions.o" \
"./BSP/Task/task.o" \
"./BSP/Timer/timer.o" \
"./BSP/eMPL/inv_mpu.o" \
"./BSP/eMPL/inv_mpu_dmp_motion_driver.o" \
$(GEN_CMDS__FLAG) \
-Wl,-ldevice.cmd.genlibs \
-Wl,-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include BSP/Buzzer/subdir_vars.mk
-include BSP/Eight_Tracking/subdir_vars.mk
-include BSP/subdir_vars.mk
-include BSP/Key/subdir_vars.mk
-include BSP/LED/subdir_vars.mk
-include BSP/MPU6050/subdir_vars.mk
-include BSP/Motor/subdir_vars.mk
-include BSP/Questions/subdir_vars.mk
-include BSP/Task/subdir_vars.mk
-include BSP/Timer/subdir_vars.mk
-include BSP/eMPL/subdir_vars.mk
-include subdir_rules.mk
-include BSP/Buzzer/subdir_rules.mk
-include BSP/Eight_Tracking/subdir_rules.mk
-include BSP/subdir_rules.mk
-include BSP/Key/subdir_rules.mk
-include BSP/LED/subdir_rules.mk
-include BSP/MPU6050/subdir_rules.mk
-include BSP/Motor/subdir_rules.mk
-include BSP/Questions/subdir_rules.mk
-include BSP/Task/subdir_rules.mk
-include BSP/Timer/subdir_rules.mk
-include BSP/eMPL/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
Competition_PJ.out 

EXE_OUTPUTS__QUOTED += \
"Competition_PJ.out" 

BIN_OUTPUTS += \
Competition_PJ.hex 

BIN_OUTPUTS__QUOTED += \
"Competition_PJ.hex" 


# All Target
all: $(OBJS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "Competition_PJ.out" secondary-outputs

# Tool invocations
Competition_PJ.out: $(OBJS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: Arm Linker'
	"C:/ti/ccstheia151/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS/bin/tiarmclang.exe" @"syscfg/device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -gdwarf-3 -Wl,-m"Competition_PJ.map" -Wl,-i"C:/ti/mspm0_sdk_2_02_00_05/source" -Wl,-i"C:/ti/mspm0_sdk_2_02_00_05/Competition_PJ_CCS/Debug/syscfg" -Wl,-i"C:/ti/ccstheia151/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS/lib" -Wl,--diag_wrap=off -Wl,--display_error_number -Wl,--warn_sections -Wl,--xml_link_info="Competition_PJ_linkInfo.xml" -Wl,--rom_model -o "Competition_PJ.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

Competition_PJ.hex: $(EXE_OUTPUTS)
	@echo 'Building secondary target: "$@"'
	@echo 'Invoking: Arm Hex Utility'
	"C:/ti/ccstheia151/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS/bin/tiarmhex.exe" --memwidth=8 --romwidth=8 --intel -o "Competition_PJ.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building secondary target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(GEN_MISC_FILES__QUOTED)$(BIN_OUTPUTS__QUOTED)$(GEN_FILES__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "empty.o" "syscfg\ti_msp_dl_config.o" "startup_mspm0g350x_ticlang.o" "BSP\Buzzer\buzzer.o" "BSP\Eight_Tracking\app_irtracking.o" "BSP\Filter.o" "BSP\delay.o" "BSP\usart.o" "BSP\Key\key.o" "BSP\LED\led.o" "BSP\MPU6050\app_mpu6050.o" "BSP\MPU6050\bsp_mpu6050.o" "BSP\Motor\app_motor.o" "BSP\Motor\app_motor_usart.o" "BSP\Motor\bsp_motor_usart.o" "BSP\Questions\questions.o" "BSP\Task\task.o" "BSP\Timer\timer.o" "BSP\eMPL\inv_mpu.o" "BSP\eMPL\inv_mpu_dmp_motion_driver.o" 
	-$(RM) "empty.d" "syscfg\ti_msp_dl_config.d" "startup_mspm0g350x_ticlang.d" "BSP\Buzzer\buzzer.d" "BSP\Eight_Tracking\app_irtracking.d" "BSP\Filter.d" "BSP\delay.d" "BSP\usart.d" "BSP\Key\key.d" "BSP\LED\led.d" "BSP\MPU6050\app_mpu6050.d" "BSP\MPU6050\bsp_mpu6050.d" "BSP\Motor\app_motor.d" "BSP\Motor\app_motor_usart.d" "BSP\Motor\bsp_motor_usart.d" "BSP\Questions\questions.d" "BSP\Task\task.d" "BSP\Timer\timer.d" "BSP\eMPL\inv_mpu.d" "BSP\eMPL\inv_mpu_dmp_motion_driver.d" 
	-$(RMDIR) $(GEN_MISC_DIRS__QUOTED)
	-@echo 'Finished clean'
	-@echo ' '

secondary-outputs: $(BIN_OUTPUTS)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets


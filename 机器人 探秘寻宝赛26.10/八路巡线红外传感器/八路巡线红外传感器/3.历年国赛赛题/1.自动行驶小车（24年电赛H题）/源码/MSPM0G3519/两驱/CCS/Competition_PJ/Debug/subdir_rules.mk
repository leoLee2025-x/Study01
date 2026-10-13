################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccstheia151/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS/bin/tiarmclang.exe" -c @"syscfg/device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"D:/CCS/Competition_PJ" -I"D:/CCS/Competition_PJ/Debug" -I"D:/ti/mspm0_sdk_2_02_00_05/source/third_party/CMSIS/Core/Include" -I"D:/ti/mspm0_sdk_2_02_00_05/source" -I"D:/CCS/Competition_PJ/BSP" -I"D:/CCS/Competition_PJ/BSP/Buzzer" -I"D:/CCS/Competition_PJ/BSP/Eight_Tracking" -I"D:/CCS/Competition_PJ/BSP/eMPL" -I"D:/CCS/Competition_PJ/BSP/Key" -I"D:/CCS/Competition_PJ/BSP/LED" -I"D:/CCS/Competition_PJ/BSP/Motor" -I"D:/CCS/Competition_PJ/BSP/MPU6050" -I"D:/CCS/Competition_PJ/BSP/Questions" -I"D:/CCS/Competition_PJ/BSP/Task" -I"D:/CCS/Competition_PJ/BSP/Timer" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)" -I"D:/CCS/Competition_PJ/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-2010712527: ../empty.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"D:/ti/ccstheia151/ccs/utils/sysconfig_1.21.1/sysconfig_cli.bat" --script "D:/CCS/Competition_PJ/empty.syscfg" -o "syscfg" -s "D:/ti/mspm0_sdk_2_02_00_05/.metadata/product.json" --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/device_linker.cmd: build-2010712527 ../empty.syscfg
syscfg/device.opt: build-2010712527
syscfg/device.cmd.genlibs: build-2010712527
syscfg/ti_msp_dl_config.c: build-2010712527
syscfg/ti_msp_dl_config.h: build-2010712527
syscfg/Event.dot: build-2010712527
syscfg: build-2010712527

syscfg/%.o: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccstheia151/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS/bin/tiarmclang.exe" -c @"syscfg/device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"D:/CCS/Competition_PJ" -I"D:/CCS/Competition_PJ/Debug" -I"D:/ti/mspm0_sdk_2_02_00_05/source/third_party/CMSIS/Core/Include" -I"D:/ti/mspm0_sdk_2_02_00_05/source" -I"D:/CCS/Competition_PJ/BSP" -I"D:/CCS/Competition_PJ/BSP/Buzzer" -I"D:/CCS/Competition_PJ/BSP/Eight_Tracking" -I"D:/CCS/Competition_PJ/BSP/eMPL" -I"D:/CCS/Competition_PJ/BSP/Key" -I"D:/CCS/Competition_PJ/BSP/LED" -I"D:/CCS/Competition_PJ/BSP/Motor" -I"D:/CCS/Competition_PJ/BSP/MPU6050" -I"D:/CCS/Competition_PJ/BSP/Questions" -I"D:/CCS/Competition_PJ/BSP/Task" -I"D:/CCS/Competition_PJ/BSP/Timer" -gdwarf-3 -MMD -MP -MF"syscfg/$(basename $(<F)).d_raw" -MT"$(@)" -I"D:/CCS/Competition_PJ/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0g351x_ticlang.o: D:/ti/mspm0_sdk_2_02_00_05/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g351x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccstheia151/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS/bin/tiarmclang.exe" -c @"syscfg/device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"D:/CCS/Competition_PJ" -I"D:/CCS/Competition_PJ/Debug" -I"D:/ti/mspm0_sdk_2_02_00_05/source/third_party/CMSIS/Core/Include" -I"D:/ti/mspm0_sdk_2_02_00_05/source" -I"D:/CCS/Competition_PJ/BSP" -I"D:/CCS/Competition_PJ/BSP/Buzzer" -I"D:/CCS/Competition_PJ/BSP/Eight_Tracking" -I"D:/CCS/Competition_PJ/BSP/eMPL" -I"D:/CCS/Competition_PJ/BSP/Key" -I"D:/CCS/Competition_PJ/BSP/LED" -I"D:/CCS/Competition_PJ/BSP/Motor" -I"D:/CCS/Competition_PJ/BSP/MPU6050" -I"D:/CCS/Competition_PJ/BSP/Questions" -I"D:/CCS/Competition_PJ/BSP/Task" -I"D:/CCS/Competition_PJ/BSP/Timer" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)" -I"D:/CCS/Competition_PJ/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '



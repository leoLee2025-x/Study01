################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
BSP/Task/%.o: ../BSP/Task/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccstheia151/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS/bin/tiarmclang.exe" -c @"syscfg/device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"D:/CCS/Competition_PJ" -I"D:/CCS/Competition_PJ/Debug" -I"D:/ti/mspm0_sdk_2_02_00_05/source/third_party/CMSIS/Core/Include" -I"D:/ti/mspm0_sdk_2_02_00_05/source" -I"D:/CCS/Competition_PJ/BSP" -I"D:/CCS/Competition_PJ/BSP/Buzzer" -I"D:/CCS/Competition_PJ/BSP/Eight_Tracking" -I"D:/CCS/Competition_PJ/BSP/eMPL" -I"D:/CCS/Competition_PJ/BSP/Key" -I"D:/CCS/Competition_PJ/BSP/LED" -I"D:/CCS/Competition_PJ/BSP/Motor" -I"D:/CCS/Competition_PJ/BSP/MPU6050" -I"D:/CCS/Competition_PJ/BSP/Questions" -I"D:/CCS/Competition_PJ/BSP/Task" -I"D:/CCS/Competition_PJ/BSP/Timer" -gdwarf-3 -MMD -MP -MF"BSP/Task/$(basename $(<F)).d_raw" -MT"$(@)" -I"D:/CCS/Competition_PJ/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '



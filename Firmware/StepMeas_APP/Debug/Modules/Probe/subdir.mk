################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Modules/Probe/voltage_current_probe.c 

OBJS += \
./Modules/Probe/voltage_current_probe.o 

C_DEPS += \
./Modules/Probe/voltage_current_probe.d 


# Each subdirectory must supply rules for building sources it contributes
Modules/Probe/voltage_current_probe.o: ../Modules/Probe/voltage_current_probe.c Modules/Probe/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/rousa/Documents/jablotron/stepper_measurement/git/Firmware/StepMeas_APP/Core/Common" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Modules-2f-Probe

clean-Modules-2f-Probe:
	-$(RM) ./Modules/Probe/voltage_current_probe.cyclo ./Modules/Probe/voltage_current_probe.d ./Modules/Probe/voltage_current_probe.o ./Modules/Probe/voltage_current_probe.su

.PHONY: clean-Modules-2f-Probe


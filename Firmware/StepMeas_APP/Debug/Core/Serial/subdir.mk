################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Serial/mb_rtu_app.c \
../Core/Serial/mb_upgrade.c \
../Core/Serial/modbus_slave.c 

OBJS += \
./Core/Serial/mb_rtu_app.o \
./Core/Serial/mb_upgrade.o \
./Core/Serial/modbus_slave.o 

C_DEPS += \
./Core/Serial/mb_rtu_app.d \
./Core/Serial/mb_upgrade.d \
./Core/Serial/modbus_slave.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Serial/%.o Core/Serial/%.su Core/Serial/%.cyclo: ../Core/Serial/%.c Core/Serial/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"C:/Users/rousa/Documents/jablotron/stepper_measurement/stepper_meas/Firmware/StepMeas_APP/Core/Common" -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/rousa/Documents/jablotron/stepper_measurement/stepper_meas/Firmware/StepMeas_APP/Core/Serial" -I"C:/Users/rousa/Documents/jablotron/stepper_measurement/stepper_meas/Firmware/StepMeas_APP/Modules/Probe" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Serial

clean-Core-2f-Serial:
	-$(RM) ./Core/Serial/mb_rtu_app.cyclo ./Core/Serial/mb_rtu_app.d ./Core/Serial/mb_rtu_app.o ./Core/Serial/mb_rtu_app.su ./Core/Serial/mb_upgrade.cyclo ./Core/Serial/mb_upgrade.d ./Core/Serial/mb_upgrade.o ./Core/Serial/mb_upgrade.su ./Core/Serial/modbus_slave.cyclo ./Core/Serial/modbus_slave.d ./Core/Serial/modbus_slave.o ./Core/Serial/modbus_slave.su

.PHONY: clean-Core-2f-Serial


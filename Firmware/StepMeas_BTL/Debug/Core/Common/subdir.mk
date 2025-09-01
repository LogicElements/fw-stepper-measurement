################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Common/bootloader.c \
../Core/Common/system_msp.c 

OBJS += \
./Core/Common/bootloader.o \
./Core/Common/system_msp.o 

C_DEPS += \
./Core/Common/bootloader.d \
./Core/Common/system_msp.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Common/%.o Core/Common/%.su Core/Common/%.cyclo: ../Core/Common/%.c Core/Common/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32F446xx -DUSE_HAL_DRIVER -DMODBUS_UPGRADE -DDEBUG -c -I../Core/Inc -I../Core/Common -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Common

clean-Core-2f-Common:
	-$(RM) ./Core/Common/bootloader.cyclo ./Core/Common/bootloader.d ./Core/Common/bootloader.o ./Core/Common/bootloader.su ./Core/Common/system_msp.cyclo ./Core/Common/system_msp.d ./Core/Common/system_msp.o ./Core/Common/system_msp.su

.PHONY: clean-Core-2f-Common


################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/SYSTEM/sys/sys.c 

OBJS += \
./Drivers/SYSTEM/sys/sys.o 

C_DEPS += \
./Drivers/SYSTEM/sys/sys.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/SYSTEM/sys/%.o Drivers/SYSTEM/sys/%.su Drivers/SYSTEM/sys/%.cyclo: ../Drivers/SYSTEM/sys/%.c Drivers/SYSTEM/sys/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -DFOR_TEST -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-SYSTEM-2f-sys

clean-Drivers-2f-SYSTEM-2f-sys:
	-$(RM) ./Drivers/SYSTEM/sys/sys.cyclo ./Drivers/SYSTEM/sys/sys.d ./Drivers/SYSTEM/sys/sys.o ./Drivers/SYSTEM/sys/sys.su

.PHONY: clean-Drivers-2f-SYSTEM-2f-sys


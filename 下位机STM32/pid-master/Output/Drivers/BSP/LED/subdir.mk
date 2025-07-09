################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/LED/led.c 

OBJS += \
./Drivers/BSP/LED/led.o 

C_DEPS += \
./Drivers/BSP/LED/led.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/LED/%.o Drivers/BSP/LED/%.su Drivers/BSP/LED/%.cyclo: ../Drivers/BSP/LED/%.c Drivers/BSP/LED/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -DFOR_TEST -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-LED

clean-Drivers-2f-BSP-2f-LED:
	-$(RM) ./Drivers/BSP/LED/led.cyclo ./Drivers/BSP/LED/led.d ./Drivers/BSP/LED/led.o ./Drivers/BSP/LED/led.su

.PHONY: clean-Drivers-2f-BSP-2f-LED


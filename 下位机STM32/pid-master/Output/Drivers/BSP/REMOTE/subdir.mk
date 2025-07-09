################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/REMOTE/remote.c 

OBJS += \
./Drivers/BSP/REMOTE/remote.o 

C_DEPS += \
./Drivers/BSP/REMOTE/remote.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/REMOTE/%.o Drivers/BSP/REMOTE/%.su Drivers/BSP/REMOTE/%.cyclo: ../Drivers/BSP/REMOTE/%.c Drivers/BSP/REMOTE/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -DFOR_TEST -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-REMOTE

clean-Drivers-2f-BSP-2f-REMOTE:
	-$(RM) ./Drivers/BSP/REMOTE/remote.cyclo ./Drivers/BSP/REMOTE/remote.d ./Drivers/BSP/REMOTE/remote.o ./Drivers/BSP/REMOTE/remote.su

.PHONY: clean-Drivers-2f-BSP-2f-REMOTE


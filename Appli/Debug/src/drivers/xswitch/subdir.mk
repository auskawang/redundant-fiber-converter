################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/drivers/xswitch/lan96455_driver.c \
../src/drivers/xswitch/lan96455_irq.c \
../src/drivers/xswitch/lan96455_vlan.c 

OBJS += \
./src/drivers/xswitch/lan96455_driver.o \
./src/drivers/xswitch/lan96455_irq.o \
./src/drivers/xswitch/lan96455_vlan.o 

C_DEPS += \
./src/drivers/xswitch/lan96455_driver.d \
./src/drivers/xswitch/lan96455_irq.d \
./src/drivers/xswitch/lan96455_vlan.d 


# Each subdirectory must supply rules for building sources it contributes
src/drivers/xswitch/%.o src/drivers/xswitch/%.su src/drivers/xswitch/%.cyclo: ../src/drivers/xswitch/%.c src/drivers/xswitch/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32N657xx -c -I../include -I../src/app -I../src/drivers/sfp -I../src/drivers/xswitch -I../src/drivers/net -I../src/platform -I../Core/Inc -I../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-src-2f-drivers-2f-xswitch

clean-src-2f-drivers-2f-xswitch:
	-$(RM) ./src/drivers/xswitch/lan96455_driver.cyclo ./src/drivers/xswitch/lan96455_driver.d ./src/drivers/xswitch/lan96455_driver.o ./src/drivers/xswitch/lan96455_driver.su ./src/drivers/xswitch/lan96455_irq.cyclo ./src/drivers/xswitch/lan96455_irq.d ./src/drivers/xswitch/lan96455_irq.o ./src/drivers/xswitch/lan96455_irq.su ./src/drivers/xswitch/lan96455_vlan.cyclo ./src/drivers/xswitch/lan96455_vlan.d ./src/drivers/xswitch/lan96455_vlan.o ./src/drivers/xswitch/lan96455_vlan.su

.PHONY: clean-src-2f-drivers-2f-xswitch


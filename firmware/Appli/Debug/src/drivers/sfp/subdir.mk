################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/drivers/sfp/sfp_driver.c \
../src/drivers/sfp/sfp_gpio.c 

OBJS += \
./src/drivers/sfp/sfp_driver.o \
./src/drivers/sfp/sfp_gpio.o 

C_DEPS += \
./src/drivers/sfp/sfp_driver.d \
./src/drivers/sfp/sfp_gpio.d 


# Each subdirectory must supply rules for building sources it contributes
src/drivers/sfp/%.o src/drivers/sfp/%.su src/drivers/sfp/%.cyclo: ../src/drivers/sfp/%.c src/drivers/sfp/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32N657xx -c -I../include -I../src/app -I../src/drivers/sfp -I../src/drivers/xswitch -I../src/drivers/net -I../src/platform -I../Core/Inc -I../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-src-2f-drivers-2f-sfp

clean-src-2f-drivers-2f-sfp:
	-$(RM) ./src/drivers/sfp/sfp_driver.cyclo ./src/drivers/sfp/sfp_driver.d ./src/drivers/sfp/sfp_driver.o ./src/drivers/sfp/sfp_driver.su ./src/drivers/sfp/sfp_gpio.cyclo ./src/drivers/sfp/sfp_gpio.d ./src/drivers/sfp/sfp_gpio.o ./src/drivers/sfp/sfp_gpio.su

.PHONY: clean-src-2f-drivers-2f-sfp


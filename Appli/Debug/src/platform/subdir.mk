################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/platform/bxp_clock.c \
../src/platform/bxp_gpio.c \
../src/platform/bxp_i2c.c \
../src/platform/bxp_spi.c 

OBJS += \
./src/platform/bxp_clock.o \
./src/platform/bxp_gpio.o \
./src/platform/bxp_i2c.o \
./src/platform/bxp_spi.o 

C_DEPS += \
./src/platform/bxp_clock.d \
./src/platform/bxp_gpio.d \
./src/platform/bxp_i2c.d \
./src/platform/bxp_spi.d 


# Each subdirectory must supply rules for building sources it contributes
src/platform/%.o src/platform/%.su src/platform/%.cyclo: ../src/platform/%.c src/platform/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32N657xx -c -I../include -I../src/app -I../src/drivers/sfp -I../src/drivers/xswitch -I../src/drivers/net -I../src/platform -I../Core/Inc -I../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-src-2f-platform

clean-src-2f-platform:
	-$(RM) ./src/platform/bxp_clock.cyclo ./src/platform/bxp_clock.d ./src/platform/bxp_clock.o ./src/platform/bxp_clock.su ./src/platform/bxp_gpio.cyclo ./src/platform/bxp_gpio.d ./src/platform/bxp_gpio.o ./src/platform/bxp_gpio.su ./src/platform/bxp_i2c.cyclo ./src/platform/bxp_i2c.d ./src/platform/bxp_i2c.o ./src/platform/bxp_i2c.su ./src/platform/bxp_spi.cyclo ./src/platform/bxp_spi.d ./src/platform/bxp_spi.o ./src/platform/bxp_spi.su

.PHONY: clean-src-2f-platform


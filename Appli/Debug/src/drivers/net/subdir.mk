################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/drivers/net/eth_rgmii.c \
../src/drivers/net/mgmt_frame.c 

OBJS += \
./src/drivers/net/eth_rgmii.o \
./src/drivers/net/mgmt_frame.o 

C_DEPS += \
./src/drivers/net/eth_rgmii.d \
./src/drivers/net/mgmt_frame.d 


# Each subdirectory must supply rules for building sources it contributes
src/drivers/net/%.o src/drivers/net/%.su src/drivers/net/%.cyclo: ../src/drivers/net/%.c src/drivers/net/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32N657xx -c -I../include -I../src/app -I../src/drivers/sfp -I../src/drivers/xswitch -I../src/drivers/net -I../src/platform -I../Core/Inc -I../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-src-2f-drivers-2f-net

clean-src-2f-drivers-2f-net:
	-$(RM) ./src/drivers/net/eth_rgmii.cyclo ./src/drivers/net/eth_rgmii.d ./src/drivers/net/eth_rgmii.o ./src/drivers/net/eth_rgmii.su ./src/drivers/net/mgmt_frame.cyclo ./src/drivers/net/mgmt_frame.d ./src/drivers/net/mgmt_frame.o ./src/drivers/net/mgmt_frame.su

.PHONY: clean-src-2f-drivers-2f-net


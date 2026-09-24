################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/app/app_init.c \
../src/app/diagnostics.c \
../src/app/optical_health.c \
../src/app/path_manager.c \
../src/app/peer_comm.c 

OBJS += \
./src/app/app_init.o \
./src/app/diagnostics.o \
./src/app/optical_health.o \
./src/app/path_manager.o \
./src/app/peer_comm.o 

C_DEPS += \
./src/app/app_init.d \
./src/app/diagnostics.d \
./src/app/optical_health.d \
./src/app/path_manager.d \
./src/app/peer_comm.d 


# Each subdirectory must supply rules for building sources it contributes
src/app/%.o src/app/%.su src/app/%.cyclo: ../src/app/%.c src/app/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32N657xx -c -I../include -I../src/app -I../src/drivers/sfp -I../src/drivers/xswitch -I../src/drivers/net -I../src/platform -I../Core/Inc -I../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-src-2f-app

clean-src-2f-app:
	-$(RM) ./src/app/app_init.cyclo ./src/app/app_init.d ./src/app/app_init.o ./src/app/app_init.su ./src/app/diagnostics.cyclo ./src/app/diagnostics.d ./src/app/diagnostics.o ./src/app/diagnostics.su ./src/app/optical_health.cyclo ./src/app/optical_health.d ./src/app/optical_health.o ./src/app/optical_health.su ./src/app/path_manager.cyclo ./src/app/path_manager.d ./src/app/path_manager.o ./src/app/path_manager.su ./src/app/peer_comm.cyclo ./src/app/peer_comm.d ./src/app/peer_comm.o ./src/app/peer_comm.su

.PHONY: clean-src-2f-app


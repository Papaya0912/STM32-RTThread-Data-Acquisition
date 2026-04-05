################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/RealThread_RTOS_RT-Thread/bsp/_template/cubemx_config/board.c 

OBJS += \
./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/bsp/_template/cubemx_config/board.o 

C_DEPS += \
./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/bsp/_template/cubemx_config/board.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/RealThread_RTOS_RT-Thread/bsp/_template/cubemx_config/%.o Middlewares/Third_Party/RealThread_RTOS_RT-Thread/bsp/_template/cubemx_config/%.su Middlewares/Third_Party/RealThread_RTOS_RT-Thread/bsp/_template/cubemx_config/%.cyclo: ../Middlewares/Third_Party/RealThread_RTOS_RT-Thread/bsp/_template/cubemx_config/%.c Middlewares/Third_Party/RealThread_RTOS_RT-Thread/bsp/_template/cubemx_config/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../RT-Thread -I../Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/inc/ -I../Middlewares/Third_Party/RealThread_RTOS_RT-Thread/include/ -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-Third_Party-2f-RealThread_RTOS_RT-2d-Thread-2f-bsp-2f-_template-2f-cubemx_config

clean-Middlewares-2f-Third_Party-2f-RealThread_RTOS_RT-2d-Thread-2f-bsp-2f-_template-2f-cubemx_config:
	-$(RM) ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/bsp/_template/cubemx_config/board.cyclo ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/bsp/_template/cubemx_config/board.d ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/bsp/_template/cubemx_config/board.o ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/bsp/_template/cubemx_config/board.su

.PHONY: clean-Middlewares-2f-Third_Party-2f-RealThread_RTOS_RT-2d-Thread-2f-bsp-2f-_template-2f-cubemx_config


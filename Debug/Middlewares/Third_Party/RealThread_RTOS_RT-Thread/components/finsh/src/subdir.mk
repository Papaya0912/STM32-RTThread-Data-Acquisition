################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/cmd.c \
../Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh.c \
../Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh_file.c \
../Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh_parse.c \
../Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/shell.c 

OBJS += \
./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/cmd.o \
./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh.o \
./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh_file.o \
./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh_parse.o \
./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/shell.o 

C_DEPS += \
./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/cmd.d \
./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh.d \
./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh_file.d \
./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh_parse.d \
./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/shell.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/%.o Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/%.su Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/%.cyclo: ../Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/%.c Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../RT-Thread -I../Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/inc/ -I../Middlewares/Third_Party/RealThread_RTOS_RT-Thread/include/ -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-Third_Party-2f-RealThread_RTOS_RT-2d-Thread-2f-components-2f-finsh-2f-src

clean-Middlewares-2f-Third_Party-2f-RealThread_RTOS_RT-2d-Thread-2f-components-2f-finsh-2f-src:
	-$(RM) ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/cmd.cyclo ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/cmd.d ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/cmd.o ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/cmd.su ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh.cyclo ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh.d ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh.o ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh.su ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh_file.cyclo ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh_file.d ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh_file.o ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh_file.su ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh_parse.cyclo ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh_parse.d ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh_parse.o ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/msh_parse.su ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/shell.cyclo ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/shell.d ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/shell.o ./Middlewares/Third_Party/RealThread_RTOS_RT-Thread/components/finsh/src/shell.su

.PHONY: clean-Middlewares-2f-Third_Party-2f-RealThread_RTOS_RT-2d-Thread-2f-components-2f-finsh-2f-src


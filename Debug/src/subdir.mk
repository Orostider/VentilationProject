################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ABBcontroller.cpp \
../src/DigitalIoPin.cpp \
../src/I2C.cpp \
../src/LiquidCrystal.cpp \
../src/ModbusMaster.cpp \
../src/SerialPort.cpp \
../src/cr_cpp_config.cpp \
../src/cr_startup_lpc15xx.cpp \
../src/main.cpp 

C_SRCS += \
../src/ITM_write.c \
../src/crp.c \
../src/retarget_itm.c \
../src/sysinit.c 

OBJS += \
./src/ABBcontroller.o \
./src/DigitalIoPin.o \
./src/I2C.o \
./src/ITM_write.o \
./src/LiquidCrystal.o \
./src/ModbusMaster.o \
./src/SerialPort.o \
./src/cr_cpp_config.o \
./src/cr_startup_lpc15xx.o \
./src/crp.o \
./src/main.o \
./src/retarget_itm.o \
./src/sysinit.o 

CPP_DEPS += \
./src/ABBcontroller.d \
./src/DigitalIoPin.d \
./src/I2C.d \
./src/LiquidCrystal.d \
./src/ModbusMaster.d \
./src/SerialPort.d \
./src/cr_cpp_config.d \
./src/cr_startup_lpc15xx.d \
./src/main.d 

C_DEPS += \
./src/ITM_write.d \
./src/crp.d \
./src/retarget_itm.d \
./src/sysinit.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -D__NEWLIB__ -DDEBUG -D__CODE_RED -DCORE_M3 -D__USE_LPCOPEN -DCPP_USE_HEAP -D__LPC15XX__ -I"C:\Users\Tommi\Documents\LPCXpresso_8.2.2_650\workspace\lpc_board_nxp_lpcxpresso_1549\inc" -I"C:\Users\Tommi\Documents\LPCXpresso_8.2.2_650\workspace\lpc_chip_15xx\inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__NEWLIB__ -DDEBUG -D__CODE_RED -DCORE_M3 -D__USE_LPCOPEN -DCPP_USE_HEAP -D__LPC15XX__ -I"C:\Users\Tommi\Documents\LPCXpresso_8.2.2_650\workspace\lpc_board_nxp_lpcxpresso_1549\inc" -I"C:\Users\Tommi\Documents\LPCXpresso_8.2.2_650\workspace\lpc_chip_15xx\inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



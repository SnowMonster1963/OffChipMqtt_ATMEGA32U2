################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Descriptors.cpp \
../NeoPixel.cpp \
../NeoPixelAllOn.cpp \
../NeoPixelCandle.cpp \
../NeoPixelConfig.cpp \
../NeoPixelTheaterChase.cpp \
../NeoPixelTwinkle.cpp \
../NeoPixels.cpp \
../OffChipMqtt.cpp \
../main.cpp 

OBJS += \
./Descriptors.o \
./NeoPixel.o \
./NeoPixelAllOn.o \
./NeoPixelCandle.o \
./NeoPixelConfig.o \
./NeoPixelTheaterChase.o \
./NeoPixelTwinkle.o \
./NeoPixels.o \
./OffChipMqtt.o \
./main.o 

CPP_DEPS += \
./Descriptors.d \
./NeoPixel.d \
./NeoPixelAllOn.d \
./NeoPixelCandle.d \
./NeoPixelConfig.d \
./NeoPixelTheaterChase.d \
./NeoPixelTwinkle.d \
./NeoPixels.d \
./OffChipMqtt.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I"C:\siriusxm\data\workspaces\playing\AtmelLibrary" -I"C:\siriusxm\data\workspaces\playing\USBLibrary" -DF_USB=16000000 -Wall -g2 -gstabs -O2 -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -funsigned-char -funsigned-bitfields -fno-exceptions -fpermissive -mmcu=atmega32u2 -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



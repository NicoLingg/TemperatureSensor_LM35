###############################################################################
# Makefile for the project Projekt
###############################################################################

## General Flags
PROJECT = Projekt
MCU = at90usb1287
TARGET = Projekt.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=16000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=Projekt.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = os_cpu_a.o os_cpu_c.o os_dbg.o uCOS_II.o TemperatureSensor_LM35.o lc798x.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) Projekt.hex Projekt.eep Projekt.lss size

## Compile
os_cpu_a.o: ../os_cpu_a.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

os_cpu_c.o: ../os_cpu_c.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

os_dbg.o: ../os_dbg.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

uCOS_II.o: ../uCOS_II.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

TemperatureSensor_LM35.o: ../TemperatureSensor_LM35.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

lc798x.o: ../lc798x.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) Projekt.elf dep/* Projekt.hex Projekt.eep Projekt.lss Projekt.map


## Other dependencies
-include $(shell mkdir dep 2>NUL) $(wildcard dep/*)


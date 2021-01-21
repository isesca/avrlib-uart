TARGET=avrlib-uart
MCU=atmega328p
SOURCES=main.c uart.c

PROGRAMMER=stk500v2
PORT=-P COM4
BAUD=

#Ab hier nichts verändern
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-c -Os
LDFLAGS=


all: hex eeprom

hex: $(TARGET).hex

eeprom: $(TARGET)_eeprom.hex

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -O ihex -j .data -j .text $(TARGET).elf $(TARGET).hex

$(TARGET)_eeprom.hex: $(TARGET).elf
	avr-objcopy -O ihex -j .eeprom --change-section-lma .eeprom=1 $(TARGET).elf $(TARGET)_eeprom.hex
	make size

$(TARGET).elf: $(OBJECTS)
	avr-gcc-5.3.0 $(LDFLAGS) -mmcu=$(MCU) $(OBJECTS) -o $(TARGET).elf

.c.o:
	avr-gcc-5.3.0 $(CFLAGS) -mmcu=$(MCU) $< -o $@

size:
	avr-size --mcu=$(MCU) -C $(TARGET).elf

program:
	avrdude -p$(MCU) $(PORT) $(BAUD) -c$(PROGRAMMER) -Uflash:w:$(TARGET).hex:a

clean:
	rm -rf *.o
	rm -rf *.elf
	rm -rf *.hex
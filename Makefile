#ALTERNATE_CORE = tiny
#BOARD_TAG = attiny85at8
#
#ISP_PROG = usbasp
#OBJDIR = build
#
#include /usr/share/arduino/Arduino.mk

baud=19200
avrType=attiny85
avrFreq=8000000 # 8 Mhz
programmerType=usbasp
objDir=build

cflags=-DF_CPU=$(avrFreq) -mmcu=$(avrType) -Wall -Werror -Wextra -Os
objects=$(patsubst %.c,%.o,$(wildcard *.c))

.PHONY: flash clean

all: main.hex

%.o: %.c
	mkdir -p $(objDir)
	avr-gcc $(cflags) -c $< -o $@

main.elf: $(objects)
	avr-gcc $(cflags) -o $@ $^

main.hex: main.elf
	avr-objcopy -j .text -j .data -O ihex $^ $@

flash: main.hex
	avrdude -p$(avrType) -c$(programmerType) -b$(baud) -v -U flash:w:$<

clean:
	rm -f *.o


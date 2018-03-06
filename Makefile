BAUD=19200
MCU=attiny85
FREQ=8000000 # 8 Mhz
PROGRAMMER=usbasp

CC=avr-gcc
CFLAGS=-DF_CPU=${FREQ} -mmcu=${MCU} -Wall -Werror -Wextra -Os

SRC=${wildcard src/*.c}
OBJ=${SRC:src/%.c=obj/%.o}
LIB=${wildcard lib/*/*.o}

.PHONY: all flash clean

all: main.hex

obj/%.o: src/%.c
	mkdir -p obj
	${CC} ${CFLAGS} -I./lib -c $< -o $@

main.elf: ${OBJ}
	${CC} ${CFLAGS} ${LIB} -o $@ $^

main.hex: main.elf
	avr-objcopy -j .text -j .data -O ihex $^ $@

flash: main.hex
	avrdude -p${MCU} -c${PROGRAMMER} -b${BAUD} -V -v -U flash:w:$<

clean:
	rm -rf obj *.elf *.hex


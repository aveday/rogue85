BAUD=19200
MCU=attiny85
FREQ=8000000 # 8 Mhz
PROGRAMMER=usbasp

CC=avr-gcc
CFLAGS=-DF_CPU=${FREQ} -mmcu=${MCU} -Wall -Werror -Wfatal-errors -Wextra -Os

SRC=${wildcard src/*.c}
OBJ=${SRC:src/%.c=obj/%.o}
LIB=${wildcard lib/*/*.o}

.PHONY: all flash clean

all: main.hex
	@avr-size --mcu=${MCU} -C main.elf

obj/%.o: src/%.c
	@echo CC $@
	@mkdir -p obj
	@${CC} ${CFLAGS} -I./lib -c $< -o $@

main.elf: ${OBJ}
	@echo LD $@
	@${CC} ${CFLAGS} ${LIB} -o $@ $^

main.hex: main.elf
	@avr-objcopy -j .text -j .data -O ihex $^ $@

flash: main.hex
	@avrdude -p${MCU} -c${PROGRAMMER} -b${BAUD} -V -U flash:w:$<
	@avr-size --mcu=${MCU} -C main.elf

clean:
	@rm -rf obj *.elf *.hex


#include <avr/pgmspace.h>

typedef uint8_t sprite_t[8];

void init_graphics() {
  _delay_ms(50);
  ssd1306_init();
  _delay_ms(50);
  ssd1306_fill(0);
}

void draw_sprite(sprite_t sprite, uint8_t x, uint8_t y) {
  ssd1306_setpos(x * 8, y);
  ssd1306_send_data_start();
  for (uint8_t i = 0; i < 8; ++i)
    ssd1306_send_byte(sprite[i]);
  ssd1306_send_data_stop();
}

void draw_bar(
    const uint8_t length,
    const uint8_t fill)
{
	ssd1306_send_data_start();
  ssd1306_send_byte(0b01111110);
  for (uint8_t i = 0; i < fill; ++i)
    ssd1306_send_byte(0b01111110);
  for (uint8_t i = 0; i < length - fill; ++i)
    ssd1306_send_byte(0b01000010);
  ssd1306_send_byte(0b01111110);
	ssd1306_send_data_stop();
}

const sprite_t brick_s PROGMEM = {
  0b01010101,
  0b01110111,
  0b01010101,
  0b11011101,
  0b01010101,
  0b01110111,
  0b01010101,
  0b11011101,
};

const sprite_t player_s PROGMEM= {
  0b00000000,
  0b00010000,
  0b11001000,
  0b00111011,
  0b00111011,
  0b11001000,
  0b00010000,
  0b00000000,
};

const sprite_t skeleton_s PROGMEM = {
  0b00011110,
  0b00100000,
  0b10010010,
  0b01101101,
  0b01001011,
  0b01101101,
  0b10010010,
  0b00100000 
};

const sprite_t rat_s PROGMEM = {
  0b00000000,
  0b00100100,
  0b00011100,
  0b00111100,
  0b00011010,
  0b01011000,
  0b01010000,
  0b00100000
};

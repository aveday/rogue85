#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <avr/pgmspace.h>

typedef const uint8_t sprite_t[8];

void init_graphics() {
  _delay_ms(50);
  ssd1306_init();

  _delay_ms(50);

  ssd1306_setpos(0, 0);
  ssd1306_send_data_start();
  for (int i = 0; i < WIDTH * 8*(HEIGHT+2); ++i)
    ssd1306_send_byte(0);

  ssd1306_send_data_stop();
}

void draw_sprite(sprite_t sprite) {
  ssd1306_send_data_start();
  for (uint8_t i = 0; i < 8; ++i) {
    ssd1306_send_byte( pgm_read_byte_near(sprite + i) );
  }
  ssd1306_send_data_stop();
}

void draw_bar(
    const uint8_t length,
    const uint8_t fill)
{
	ssd1306_send_data_start();
  ssd1306_send_byte(0);
  ssd1306_send_byte(0b01111110);
  ssd1306_send_byte(0b01000010);
  for (uint8_t i = 0; i < fill; ++i)
    ssd1306_send_byte(0b01011010);
  for (uint8_t i = 0; i < length - fill; ++i)
    ssd1306_send_byte(0b01000010);
  ssd1306_send_byte(0b01000010);
  ssd1306_send_byte(0b1111110);
  ssd1306_send_byte(0);
	ssd1306_send_data_stop();
}

#endif

#ifndef DEBUG_H
#define DEBUG_H

#include "input.h"

uint8_t pressed_s[] = {
  0b00000000,
  0b00111100,
  0b01111110,
  0b01111110,
  0b01111110,
  0b01111110,
  0b00111100,
  0b00000000
};

uint8_t unpressed_s[] = {
  0b00000000,
  0b00111100,
  0b01000010,
  0b01000010,
  0b01000010,
  0b01000010,
  0b00111100,
  0b00000000
};

void draw_buttons(uint8_t input) {
  draw_sprite((input & UP) ? pressed_s : unpressed_s,  1, 2);
  draw_sprite((input & DOWN) ? pressed_s : unpressed_s,  1, 4);
  draw_sprite((input & LEFT) ? pressed_s : unpressed_s,  0, 3);
  draw_sprite((input & RIGHT) ? pressed_s : unpressed_s,  2, 3);
  draw_sprite((input & A) ? pressed_s : unpressed_s, 14, 4);
  draw_sprite((input & B) ? pressed_s : unpressed_s, 15, 3);
  draw_sprite((input & X) ? pressed_s : unpressed_s, 13, 3);
  draw_sprite((input & Y) ? pressed_s : unpressed_s, 14, 2);
}

void draw_voltages(int left_voltage, int right_voltage) {
  ssd1306_setpos(0, 0);
  ssd1306_numdecp_font6x8(left_voltage);
  ssd1306_setpos(98, 0);
  ssd1306_numdecp_font6x8(right_voltage);
}

void draw_load(int dt) {
  uint8_t load = dt / 10;
  ssd1306_setpos(48, 0);
  ssd1306_numdecp_font6x8(load);
}

void debug(char* s, uint8_t i) {
  ssd1306_setpos(60, 0);
  ssd1306_string_font6x8(s);
  ssd1306_numdecp_font6x8(i);
}

#endif

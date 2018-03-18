#ifndef DEBUG_H
#define DEBUG_H

#include "ssd1306xled/ssd1306xled.h"

void debug(char* s, uint8_t i) {
  ssd1306_setpos(0, 1);
  ssd1306_string_font6x8(s);
  ssd1306_numdecp_font6x8(i);
}

#endif

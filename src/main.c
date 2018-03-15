#include <math.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "ssd1306xled/ssd1306xled.h"
#include "config.h"
#include "input.h"
#include "graphics.h"
#include "entity.h"

#ifdef DEBUG
#include "debug.h"
#endif

#define XY(x, y) ((y << 4) + x)

uint8_t turn = 0;

void draw_ui(entityId id) {
  ssd1306_setpos(0, 0);
  ssd1306_string_font6x8("HP");
  draw_bar( pgm_read_byte_near(
    &templates[entities[id].templateId].max_hp
  ), entities[id].hp);

  ssd1306_setpos(80, 1);
  ssd1306_string_font6x8("turn ");
  ssd1306_numdec_font6x8(turn);
}

void draw_level() {
  ssd1306_setpos(0, 2);
  ssd1306_send_data_start();
  for (uint8_t pos = 0; pos < WIDTH * HEIGHT; ++pos) {
    for (uint8_t b = 0; b < 8; ++b)
      if (!level[pos])
        ssd1306_send_byte(0);
      else
        ssd1306_send_byte(pgm_read_byte_near(
            TEMPLATE(level[pos]).sprite + b
        ));
  }
  ssd1306_send_data_stop();
}

void loop(entityId player) {
  // RENDER
  draw_ui(player);
  draw_level();

  for (uint8_t id = 0; id < MAX_ENTITIES; ++id) {
    if (!entities[id].hp) continue;

    void (*behaviour)(entityId) = FIELD(ptr, id, behaviour);
    if (behaviour) behaviour(id);

    if (id == player) {
      draw_level(player);
      _delay_ms(50);
    }
  }

  ++turn;
}

void add_room(uint8_t* level, uint8_t p1, uint8_t p2) {
  for (uint8_t x = (p1 & 0xF); x <= (p2 & 0xF); ++x) {
    for (uint8_t y = p1 >> 4; y <= p2 >> 4; ++y) {
      level[y*2 + x/8] |= 0b10000000 >> x%8;
    }
  }
}

void build_level() {
  uint8_t plan[12];

  for (uint8_t i = 0; i < 12; ++i) plan[i] = 0;
  add_room(plan, 0, XY(5, 4));

  for (uint8_t x = 0; x < WIDTH; ++x)
    for (uint8_t y = 0; y < HEIGHT; ++y)
      if ( !(plan[(y<<1) + (x>>3)] & 0b10000000 >> x%8) )
        add_entity(find_template(TERRAIN), XY(x, y));

  add_entity(find_template(MONSTER), 35);
}

int main() {
  MCUSR = 0x00;
  wdt_disable();

  init_graphics();
  init_input();

  // initialize entity and level arrays
  for (int i = 0; i < MAX_ENTITIES; ++i) remove_entity(i);
  for (int i = 0; i < WIDTH*HEIGHT; ++i) level[i] = 0;

  entityId player = add_entity(find_template(PLAYER), 0);
  build_level();

  while (entities[player].hp) //FIXME
    loop(player);

  wdt_enable(WDTO_2S);
  for (;;) ssd1306_string_font6x8("YOU DIED. ");
}

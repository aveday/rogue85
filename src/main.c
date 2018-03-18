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
#include "level.h"

#include "debug.h"

uint8_t turn = 0;

sprite_t selected_s = {
  0b00000010,
  0b00000100,
  0b00001100,
  0b00001100,
  0b00001100,
  0b00001100,
  0b00000100,
  0b00000010
};

void draw_ui(entityId id) {
  ssd1306_setpos(0, 0);
  ssd1306_string_font6x8("HP");
  draw_bar( pgm_read_byte_near(
    &templates[entities[id].templateId].max_hp
  ), entities[id].hp);

  uint8_t inventory_x = 128 - 9 * INVENTORY;

  ssd1306_setpos(inventory_x, 0);
  ssd1306_send_data_start();
  for (uint8_t i = 0; i < INVENTORY; ++i) {
    uint8_t item = level[WIDTH * HEIGHT + i];
    ssd1306_send_byte(0);
    for (uint8_t b = 0; b < 8; ++b) {
      uint8_t byte = 0;
      if (entities[item].hp)
        byte = pgm_read_byte_near(TEMPLATE(item).sprite + b);
      else if (b == 3 || b == 4)
        byte = 0x18;
      ssd1306_send_byte(byte);
    }
  }
  ssd1306_setpos(inventory_x, 1);
  ssd1306_send_data_start();
  for (uint8_t i = 0; i < INVENTORY; ++i) {
    ssd1306_send_byte(0);
    for (uint8_t b = 0; b < 8; ++b)
      ssd1306_send_byte(i == selected ? selected_s[b] : 0);
  }
  ssd1306_send_data_stop();
}

void draw_level() {
  ssd1306_setpos(0, 2);
  ssd1306_send_data_start();
  for (uint8_t pos = 0; pos < WIDTH * HEIGHT; ++pos) {
    for (uint8_t b = 0; b < 8; ++b) {
      uint8_t byte = 0;
      if (!(pos % WIDTH || b)  || (pos % WIDTH == WIDTH-1 && b == 7))
        byte |= 0b10101010;
      if (pos < WIDTH && b % 4)
        byte |= 1;
      else if (pos / WIDTH == HEIGHT - 1 && b % 4)
        byte |= 1<<7;
      if (!visible(pos))
        byte |= 1 << (b*5 % 8);
      else if (level[pos])
        byte |= pgm_read_byte_near(TEMPLATE(level[pos]).sprite + b);

      ssd1306_send_byte(byte);
    }
  }
  ssd1306_send_data_stop();
}

void loop(entityId player) {
  // RENDER
  draw_ui(player);
  draw_level();

  for (uint8_t id = 0; id < MAX_ENTITIES; ++id) {
    if (!entities[id].hp || FLAG(id, WALL) || FLAG(id, DOOR)) continue;

    if (id == player)
      draw_level(player);

    void (*behaviour)(entityId) = FIELD(ptr, id, behaviour);
    if (behaviour) behaviour(id);
  }

  ++turn;
}

int main() {
  MCUSR = 0x00;
  wdt_disable();

  init_graphics();
  init_input();

  // initialize inventory
  for (uint8_t i = 0; i < INVENTORY; ++i)
    level[WIDTH * HEIGHT + i] = 0;

  // initialize entity array
  for (entityId id = 0; id < MAX_ENTITIES; ++id)
    remove_entity(id);

  build_level(1);

  entityId player = find_entity(PLAYER);

  while (entities[player].hp) //FIXME
    loop(player);

  wdt_enable(WDTO_2S);
  for (;;) ssd1306_string_font6x8("YOU DIED. ");
}

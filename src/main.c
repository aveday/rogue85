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
    if (!entities[id].hp || FLAG(id, WALL) || FLAG(id, DOOR)) continue;
    draw_level();

    void (*behaviour)(entityId) = FIELD(ptr, id, behaviour);
    if (behaviour) behaviour(id);

    if (id == player) {
      draw_level(player);
    }
  }

  ++turn;
}

int main() {
  MCUSR = 0x00;
  wdt_disable();

  init_graphics();
  init_input();

  // initialize entity and level arrays
  for (entityId id = 0; id < MAX_ENTITIES; ++id)
    remove_entity(id);

  build_level();

  entityId player = find_entity(PLAYER);

  while (entities[player].hp) //FIXME
    loop(player);

  wdt_enable(WDTO_2S);
  for (;;) ssd1306_string_font6x8("YOU DIED. ");
}

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

void draw_room() {
  ssd1306_setpos(0, 2);
  ssd1306_send_data_start();
  for (int i = 0; i < 8 * WIDTH * HEIGHT; ++i) {
    entity_t e = entities[room[i/8]];
    ssd1306_send_byte(pgm_read_byte_near(
        templates[e.templateId].sprite + i%8
    ));
  }
  ssd1306_send_data_stop();
}

void loop(entityId player) {
  // RENDER
  draw_ui(player);
  draw_room();

  for (uint8_t id = 0; id < MAX_ENTITIES; ++id) {
    if (entities[id].templateId != EMPTY &&
        entities[id].templateId != INVALID)
      ((void(*)())(pgm_read_ptr_near(&(TEMPLATE(id).behaviour))))(id);
  }

  ++turn;
}

int main() {
  MCUSR = 0x00;
  wdt_disable();

  init_graphics();
  init_input();

  // initialize entity and room arrays
  for (int i = 0; i < MAX_ENTITIES; ++i) remove_entity(i);
  for (int i = 0; i < WIDTH*HEIGHT; ++i) room[i] = EMPTY;

  add_entity(EMPTY, INVALID);
  entityId player = add_entity(PLAYER, 1);
  add_entity(RAT, 30);

  while (entities[player].templateId == PLAYER)
    loop(player);

  wdt_enable(WDTO_2S);
  for (;;) ssd1306_string_font6x8("YOU DIED. ");
}

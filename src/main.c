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

#define MIN 2

uint8_t cx(uint8_t p) { return p % WIDTH; }
uint8_t cy(uint8_t p) { return p / WIDTH; }
uint8_t cxy(uint8_t x, uint8_t y) { return WIDTH * y + x; }
uint8_t cyx(uint8_t y, uint8_t x) { return WIDTH * y + x; }

void split_room(uint8_t p1, uint8_t p2) {

  uint8_t width = cx(p2) - cx(p1) + 1;
  uint8_t height= cy(p2) - cy(p1) + 1;
  bool too_narrow = width < MIN*2 + 1;
  bool too_short  = height < MIN*2 + 1;

  if (too_narrow && too_short)
    return;

  // vertical split
  uint8_t (*c1)(uint8_t) = cx;
  uint8_t (*c2)(uint8_t) = cy;
  uint8_t (*cp)(uint8_t, uint8_t) = cxy;
  bool v = too_short || (rand() % (width + height) < width && !too_narrow);
  if (!v) {
    // horizontal split
    c1 = cy;
    c2 = cx;
    cp = cyx;
  }

  uint8_t split = c1(p1) + MIN + rand() % (c1(p2) - c1(p1) + 1 - 2*MIN);

  // check if new wall would block an existing door
  for (int8_t d = -1; d <= 1; d += 2)
    if (in_bounds(cp(split, c2(d<0?p1:p2)), v?0:d, v?d:0) &&
        FLAG(level[cp(split, c2(d<0?p1:p2) + d)], DOOR))
    return;

  uint8_t door = c2(p1) + rand() % (c2(p2) - c2(p1) + 1);

  for (uint8_t n = c2(p1); n <= c2(p2); ++n) {
    add_entity(find_template(door == n ? DOOR : WALL), cp(split, n));
  }

  /*
  sprite_t block = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  //debug(v ? "V" : "H", split);
  while(!get_input(RIGHT_INPUT)) continue;
  while(get_input(RIGHT_INPUT)) continue;
  for (uint8_t n = c2(p1); n <= c2(p2); ++n)
    draw_sprite(block, cx(cp(split, n)), cy(cp(split, n)) + 2);
    */

  split_room(p1, cp(split-1, c2(p2)));
  split_room(cp(split+1, c2(p1)), p2);
}

void build_level() {

  for (int i = 0; i < WIDTH*HEIGHT; ++i)
    level[i] = 0;

  split_room(0, cxy(WIDTH - 1, HEIGHT - 1));

  // add player
  add_entity(find_template(PLAYER), 0);
  // add monsters
  add_entity(find_template(MONSTER), 35);
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

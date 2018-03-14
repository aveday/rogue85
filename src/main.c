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

void die() {
  wdt_enable(WDTO_2S);
  for (;;) ssd1306_string_font6x8("YOU DIED. ");
}

void draw_ui() {
  ssd1306_setpos(0, 0);
  ssd1306_string_font6x8("HP");
  draw_bar( pgm_read_byte_near(
    &templates[entities[PLAYER].templateId].max_hp
  ), entities[PLAYER].hp);

  ssd1306_setpos(80, 1);
  ssd1306_string_font6x8("turn ");
  ssd1306_numdec_font6x8(turn);

}

void draw_room() {
  ssd1306_setpos(0, 2);
  ssd1306_send_data_start();
  for (int i = 0; i < 8 * WIDTH * HEIGHT; ++i) {
    entity_t e = entities[room[i/8]];
    if (e.templateId == INVALID) continue;
    uint8_t line = pgm_read_byte_near(
        templates[e.templateId].sprite + i%8
    );
    ssd1306_send_byte(line);
  }
  ssd1306_send_data_stop();
}

int8_t sign(int8_t a) {
  if (a > 0) return 1;
  if (a < 0) return -1;
  return 0;
}

void take_turn(uint8_t input) {
  int8_t ix = (bool)(input & RIGHT) - (bool)(input & LEFT);
  int8_t iy = (bool)(input & DOWN) - (bool)(input & UP);

  if (query_adjacent(PLAYER, ix, iy) == EMPTY)
    move(PLAYER, ix, iy);
  else
    return;

  int8_t px = entities[PLAYER].pos % WIDTH;
  int8_t py = entities[PLAYER].pos / WIDTH;

  draw_room();
  _delay_ms(100);

  for (uint8_t id = PLAYER+1; id < MAX_ENTITIES; ++id) {
    if (entities[id].templateId == EMPTY ||
        entities[id].templateId == INVALID) continue;
    // FOLLOW
    int8_t ex = entities[id].pos % WIDTH;
    int8_t ey = entities[id].pos / WIDTH;
    int8_t dx = sign(px - ex);
    int8_t dy = sign(py - ey);
    if (query_adjacent(id, dx, dy) == EMPTY)
      move(id, dx, dy);

    // ATTACK
    uint8_t diff = entities[PLAYER].pos > entities[id].pos
      ? entities[PLAYER].pos - entities[id].pos
      : entities[id].pos - entities[PLAYER].pos;
    if (diff == 1 || diff == WIDTH || diff == HEIGHT)
      entities[PLAYER].hp--;
    if (!entities[PLAYER].hp) die();
  }
  ++turn;
}

int loop() {
  draw_ui();
  draw_room();

  uint8_t input;
  while(!(input = get_input(LEFT_INPUT) << 4)) _delay_ms(10);

  take_turn(input);
  return 0;
}

int main() {
  MCUSR = 0x00;
  wdt_disable();

  init_graphics();
  init_input(repeat = false);

  // initialize entity and room arrays
  for (int i = 0; i < MAX_ENTITIES; ++i) {
    entities[i].templateId = INVALID;
    entities[i].pos = INVALID;
    entities[i].hp = INVALID;
  }
  for (int i = 0; i < WIDTH*HEIGHT; ++i)
    room[i] = EMPTY;

  add_entity(EMPTY, INVALID);
  add_entity(PLAYER, 1);
  add_entity(RAT, 30);

  for (int id = 1; id < MAX_ENTITIES; ++id)
    room[entities[id].pos] = id;

  while(!loop()) continue;
}

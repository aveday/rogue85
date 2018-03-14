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

bool take_turn(uint8_t input) {
  int8_t dx = input & LEFT ? -1 : input & RIGHT ? 1 : 0;
  int8_t dy = input & UP ? -1 : input & DOWN ? 1 : 0;

  switch(input & 0b1111) {
    case 0: return move(PLAYER, dx, dy);
    case A: return attack(PLAYER, dx, dy);
    case B: return false;
    case X: return false;
    case Y: return false;
  }
  return false;
}

bool loop(bool held) {
  // RENDER
  draw_ui();
  draw_room();

  // HANDLE INPUT
  uint8_t left_input, right_input;
  do {
    for (;;) {
      left_input = get_input(LEFT_INPUT);
      if (!left_input) {
        held = false;
      } else if (!held) {
        right_input = get_input(RIGHT_INPUT);
        held = true;
        break;
      }
      _delay_ms(10);
    }
  } while (!take_turn((left_input << 4) + right_input));

  draw_room();
  _delay_ms(100);

  // MOVE NPCs
  int8_t px = entities[PLAYER].pos % WIDTH;
  int8_t py = entities[PLAYER].pos / WIDTH;

  for (uint8_t id = PLAYER+1; id < MAX_ENTITIES; ++id) {
    if (entities[id].templateId == EMPTY ||
        entities[id].templateId == INVALID) continue;
    // FOLLOW
    int8_t ex = entities[id].pos % WIDTH;
    int8_t ey = entities[id].pos / WIDTH;
    int8_t dx = sign(px - ex);
    int8_t dy = sign(py - ey);
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
  return held;
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
  add_entity(PLAYER, 1);
  add_entity(RAT, 30);

  bool held = false;
  for (;;) held = loop(held);
}

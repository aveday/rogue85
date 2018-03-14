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

int8_t sign(int8_t a) {
  if (a > 0) return 1;
  if (a < 0) return -1;
  return 0;
}

bool take_turn(entityId id, uint8_t input) {
  int8_t dx = input & LEFT ? -1 : input & RIGHT ? 1 : 0;
  int8_t dy = input & UP ? -1 : input & DOWN ? 1 : 0;

  switch(input & 0b1111) {
    case 0: return move(id, dx, dy);
    case A: return attack(id, dx, dy);
    case B: return false;
    case X: return false;
    case Y: return false;
  }
  return false;
}

void loop(entityId player) {
  // RENDER
  draw_ui(player);
  draw_room();

  // HANDLE INPUT
  uint8_t input = 0;
  while (get_input(LEFT_INPUT));            // await release
  while (!(input = get_input(LEFT_INPUT))); // await left input
  input <<= 4;                              // shift left input
  input += get_input(RIGHT_INPUT);          // get right state
  if (!take_turn(player, input)) return;    // only take valid turns

  draw_room();
  _delay_ms(100);

  // MOVE NPCs
  int8_t px = entities[player].pos % WIDTH;
  int8_t py = entities[player].pos / WIDTH;

  for (uint8_t id = 0; id < MAX_ENTITIES; ++id) {
    if (entities[id].templateId == EMPTY ||
        entities[id].templateId == PLAYER || //FIXME
        entities[id].templateId == INVALID) continue;
    // FOLLOW
    int8_t ex = entities[id].pos % WIDTH;
    int8_t ey = entities[id].pos / WIDTH;
    move(id, sign(px - ex), sign(py - ey));

    // ATTACK
    int8_t dx = px - entities[id].pos % WIDTH;
    int8_t dy = py - entities[id].pos / WIDTH;
    if ((abs(dx) == 1 && !dy) || (abs(dy) == 1 && !dx))
      attack(id, dx, dy);
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

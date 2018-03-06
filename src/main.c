#include <math.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>

#include "ssd1306xled/ssd1306xled.h"
#include "config.h"
#include "graphics.h"
#include "input.h"

#ifdef DEBUG
#include "debug.h"
#endif

#define WIDTH 16
#define HEIGHT 6
#define MAX_ENTITIES 30

#define EMPTY 0
#define PLAYER 1
#define SKELETON 2
#define RAT 3

uint8_t count = 0;

typedef struct {
  uint8_t pos;
  uint8_t type;
} entity_t;

entity_t* room[WIDTH*HEIGHT];

entity_t entities[MAX_ENTITIES];
uint8_t entity_count = 1;

const sprite_t *ids[] = {
  0,
  &player_s,
  &skeleton_s,
  &rat_s
};

void draw_ui() {
  ssd1306_setpos(0, 0);
  ssd1306_string_font6x8("HP");
  draw_bar(20, 7, 2, 0);

  ssd1306_setpos(0, 1);
  ssd1306_string_font6x8("MG");
  draw_bar(20, 12, 2, 1);
}

void add_entity(uint8_t type, uint8_t pos) {
  entities[entity_count].type = type;
  entities[entity_count].pos = pos;
  ++entity_count;
}

void draw_room() {
  ssd1306_setpos(0, 2);
	ssd1306_send_data_start();
  for (int i = 0; i < 8 * WIDTH * HEIGHT; ++i) {
    uint8_t line = room[i/8]->type ? pgm_read_byte_near(
      (*(ids[(room[i/8])->type])) + (i%8))
     : 0;
    ssd1306_send_byte(line);
  }
	ssd1306_send_data_stop();
}

bool can_move(uint8_t pos, int8_t dx, int8_t dy) {
  if ( pos % WIDTH + dx < 0 || pos % WIDTH + dx >= WIDTH // H out of bounds
    || pos / WIDTH + dy < 0 || pos / WIDTH + dx >= HEIGHT // V out of bounds
    || room[pos + dx + WIDTH * dy]->type) // space occupied
    return false;
  return true;
}

void move(entity_t* entity, int8_t dx, int8_t dy) {
  room[entity->pos] = &(entities[EMPTY]);
  entity->pos += dx + WIDTH * dy;
  room[entity->pos] = entity;
  draw_room();
}

int8_t sign(int8_t a) {
  if (a > 0) return 1;
  if (a < 0) return -1;
  return 0;
}

void take_turn(uint8_t input) {
  int8_t dx = (bool)(input & RIGHT) - (bool)(input & LEFT);
  int8_t dy = (bool)(input & DOWN) - (bool)(input & UP);

  if (can_move(entities[PLAYER].pos, dx, dy))
    move(&entities[PLAYER], dx, dy);

  int8_t px = entities[PLAYER].pos % WIDTH;
  int8_t py = entities[PLAYER].pos / WIDTH;

  for (uint8_t i = 2; i < entity_count; ++i) {
    int8_t ix = entities[i].pos % WIDTH;
    int8_t iy = entities[i].pos / WIDTH;
    int8_t dx = sign(px - ix);
    int8_t dy = sign(py - iy);

    if (can_move(entities[i].pos, dx, dy))
      move(&entities[i], dx, dy);
  }
}

void setup() {
  init_graphics();
  init_input(repeat = false);

  add_entity(PLAYER, 1);
  add_entity(SKELETON, 3);
  add_entity(RAT, 30);

  for (int i = 0; i < WIDTH*HEIGHT; ++i)
    room[i] = &(entities[EMPTY]);

  for (int i = 1; i < entity_count; ++i)
    room[entities[i].pos] = &(entities[i]);

  draw_ui();
  draw_room();
}

void loop() {
  ssd1306_setpos(48, 1);
  ssd1306_numdecp_font6x8(count++);

  uint8_t input;
  while(!(input = get_input())) _delay_ms(10);

  take_turn(input);
  draw_ui();
  draw_room();
}

int main() {
  setup();
  while(1) loop();
}

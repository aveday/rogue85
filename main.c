#include <avr/io.h>
#include <util/delay.h>

#include "ssd1306xled.h"
#include "config.h"
#include "graphics.h"
#include "input.h"
#include "clock.h"

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

uint8_t player_x = 0, player_y = 0;

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

void take_turn(uint8_t input) {
  room[entities[PLAYER].pos] = &(entities[EMPTY]);

  if (input & LEFT && entities[PLAYER].pos % WIDTH > 0)
    --entities[PLAYER].pos;
  if (input & RIGHT && entities[PLAYER].pos % WIDTH < WIDTH-1)
    ++entities[PLAYER].pos;
  if (input & UP && entities[PLAYER].pos / WIDTH > 0)
    entities[PLAYER].pos -= WIDTH;
  if (input & DOWN && entities[PLAYER].pos / WIDTH < HEIGHT-1)
    entities[PLAYER].pos += WIDTH;

  room[entities[PLAYER].pos] = &(entities[PLAYER]);
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

void setup() {
  init_graphics();
  init_input(repeat = false);
  time_step();

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
  ++count;
  uint8_t dt = time_step();
  uint8_t input = get_input();

  if (input) {
    take_turn(input);
    draw_ui();
    draw_room();
  }
  uint8_t load = FPS * dt / 10;
  ssd1306_setpos(48, 0);
  ssd1306_numdecp_font6x8(load);
}

int main() {
  setup();
  while(1) loop();
}

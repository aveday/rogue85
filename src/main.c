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

typedef struct {
  uint8_t corner1;
  uint8_t corner2;
} room_t;


bool split_room(room_t rooms[]) {
  uint8_t cxy(uint8_t x, uint8_t y) { return WIDTH * y + x; }
  uint8_t cyx(uint8_t y, uint8_t x) { return WIDTH * y + x; }
  
  uint8_t id;
  uint8_t c11, c12, c21, c22;
  uint8_t (*cp)(uint8_t, uint8_t);
  uint8_t split;

  // pick which room to split
  for (id = 0; id < MAX_ROOMS - 1; ++id) {
    room_t room = rooms[id];

    if (room.corner1 > room.corner2)
      continue;

    uint8_t width = room.corner2 % WIDTH - room.corner1 % WIDTH + 1;
    uint8_t height= room.corner2 / WIDTH - room.corner1 / WIDTH + 1;
    bool too_narrow = width < MIN*2 + 1;
    bool too_short  = height < MIN*2 + 1;

    if (too_narrow && too_short)
      continue;

    bool v = too_short || (rand() % (width + height) < width && !too_narrow);
    c11 = v ? room.corner1 % WIDTH : room.corner1 / WIDTH;
    c21 = v ? room.corner1 / WIDTH : room.corner1 % WIDTH;
    c12 = v ? room.corner2 % WIDTH : room.corner2 / WIDTH;
    c22 = v ? room.corner2 / WIDTH : room.corner2 % WIDTH;
    cp  = v ? cxy : cyx;

    split = c11 + MIN + rand() % (c12 - c11 + 1 - 2*MIN);

    // check if new wall would block an existing door
    bool blocking = false;
    for (int8_t d = -1; d <= 1; d += 2)
      if (in_bounds(cp(split, d<0?c21:c22), v?0:d, v?d:0) &&
          FLAG(level[cp(split, (d<0?c21:c22) + d)], DOOR))
        blocking = true;

    if (blocking)
      continue;
    break;
  }

  if (id == MAX_ROOMS - 1)
    return false;

  uint8_t next = id + 1;
  while (next < MAX_ROOMS && rooms[next].corner1 <= rooms[next].corner2)
    ++next;
  if (next == MAX_ROOMS)
    return false;

  uint8_t nc1 = cp(split + 1, c21);
  uint8_t nc2 = cp(split - 1, c22);
  if (rand() % 2) {
    rooms[next].corner1 = nc1;
    rooms[next].corner2 = rooms[id].corner2;
    rooms[id].corner2   = nc2;
  } else {
    rooms[next].corner1 = rooms[id].corner1;
    rooms[next].corner2 = nc2;
    rooms[id].corner1   = nc1;
  }

  uint8_t door = c21 + rand() % (c22 - c21 + 1);

  for (uint8_t n = c21; n <= c22; ++n) {
    add_entity(find_template(door == n ? DOOR : WALL), cp(split, n));
  }

  return true;
}

void build_level() {
  for (int i = 0; i < WIDTH*HEIGHT; ++i)
    level[i] = 0;
  
  room_t rooms[MAX_ROOMS];
  for (int i = 0; i < MAX_ROOMS; ++i) {
    // rooms unused if corner1 > corner2
    rooms[i].corner1 = 1;
    rooms[i].corner2 = 0;
  }

  rooms[0].corner1 = 0;
  rooms[0].corner2 = (WIDTH - 1) + WIDTH * (HEIGHT - 1);
  while (split_room(rooms)) continue;

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

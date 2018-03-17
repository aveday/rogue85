#ifndef LEVEL_H
#define LEVEL_H

#include "entity.h"
#include "config.h"

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
    bool too_narrow = width < MIN_X*2 + 1;
    bool too_short  = height < MIN_Y*2 + 1;

    if (too_narrow && too_short)
      continue;

    bool v = too_short || (rand() % (width + height) < width && !too_narrow);
    c11 = v ? room.corner1 % WIDTH : room.corner1 / WIDTH;
    c21 = v ? room.corner1 / WIDTH : room.corner1 % WIDTH;
    c12 = v ? room.corner2 % WIDTH : room.corner2 / WIDTH;
    c22 = v ? room.corner2 / WIDTH : room.corner2 % WIDTH;
    cp  = v ? cxy : cyx;
    uint8_t min = v ? MIN_X : MIN_Y;
    split = c11 + min + rand() % (c12 - c11 + 1 - 2*min);

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
    add_entity(door == n ? DOOR : WALL, cp(split, n));
  }

  return true;
}

void build_level(uint8_t depth) {
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
  add_entity(PLAYER, 0);

  // fill rooms
  for (uint8_t i = 0; i < MAX_ROOMS; ++i) {
    room_t room = rooms[i];
    if (room.corner1 > room.corner2 || !room.corner1 /*FIXME*/)
      continue;

    uint8_t width  = room.corner2 % WIDTH - room.corner1 % WIDTH + 1;
    uint8_t height = room.corner2 / WIDTH - room.corner1 / WIDTH + 1;

    uint8_t monster_count = 0;
    while ( rand() % 0xFF < MONSTER_SPAWN_RATE &&
            monster_count++ < depth) {

      uint8_t pos = room.corner1 + rand() % width
                                 + rand() % height * WIDTH;
      if (!level[pos])
        add_entity(MONSTER, pos);
    }

    uint8_t item_count = 0;
    while ( rand() % 0xFF < ITEM_SPAWN_RATE &&
            item_count++ < depth) {

      uint8_t pos = room.corner1 + rand() % width
                                 + rand() % height * WIDTH;
      if (!level[pos])
        add_entity(ITEM, pos);
    }
  }
}

#endif

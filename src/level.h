#ifndef LEVEL_H
#define LEVEL_H

#include "entity.h"
#include "config.h"

#define ROOM_EXISTS(i) (rooms[i].corner1 <= rooms[i].corner2)

// room flags
#define HIDDEN (1 << 0)
#define ENTRY  (1 << 1)
#define EXIT   (1 << 2)

typedef struct {
  uint8_t corner1;
  uint8_t corner2;
  uint8_t flags;
} room_t;

room_t rooms[MAX_ROOMS];

bool near_door(uint8_t pos) {
  for (int8_t d = -1; d <= 1; d += 2)
    if ((in_bounds(pos, d, 0) && FLAG(level[pos +       d], DOOR)) ||
        (in_bounds(pos, 0, d) && FLAG(level[pos + WIDTH*d], DOOR)))
      return true;
  return false;
}

bool split_room(room_t rooms[], uint8_t depth) {
  uint8_t cxy(uint8_t x, uint8_t y) { return WIDTH * y + x; }
  uint8_t cyx(uint8_t y, uint8_t x) { return WIDTH * y + x; }

  uint8_t id;
  uint8_t c11, c12, c21, c22;
  uint8_t (*cp)(uint8_t, uint8_t);
  uint8_t split;

  // pick which room to split
  for (id = 0; id < MAX_ROOMS - 1; ++id) {
    room_t room = rooms[id];

    if (!(ROOM_EXISTS(id)) || (room.flags & HIDDEN))
      continue;

    uint8_t width = room.corner2 % WIDTH - room.corner1 % WIDTH + 1;
    uint8_t height= room.corner2 / WIDTH - room.corner1 / WIDTH + 1;
    bool too_narrow = width < MIN_X*2 + 1;
    bool too_short  = height < MIN_Y*2 + 1;

    if (too_narrow && too_short)
      rooms[id].flags |= HIDDEN;

    bool v = too_short || (rand() % (width + height) < width && !too_narrow);
    c11 = v ? room.corner1 % WIDTH : room.corner1 / WIDTH;
    c21 = v ? room.corner1 / WIDTH : room.corner1 % WIDTH;
    c12 = v ? room.corner2 % WIDTH : room.corner2 / WIDTH;
    c22 = v ? room.corner2 / WIDTH : room.corner2 % WIDTH;
    cp  = v ? cxy : cyx;
    uint8_t min = v ? MIN_X : MIN_Y;
    split = c11 + min + rand() % (c12 - c11 + 1 - 2*min);

    // check if new wall would be built over existing features
    for (uint8_t n = c21; n <= c22; ++n)
      if (level[cp(split, n)])
        rooms[id].flags |= HIDDEN;

    // check if new wall would block an existing door
    if (near_door(cp(split, c21)) ||
        near_door(cp(split, c22)))
      rooms[id].flags |= HIDDEN;

    if (!(rooms[id].flags & HIDDEN))
      break;
  }

  if (id == MAX_ROOMS - 1)
    return false;

  uint8_t next = id + 1;
  while (next < MAX_ROOMS && ROOM_EXISTS(next))
    ++next;
  if (next == MAX_ROOMS)
    return false;

  rooms[next].corner1 = cp(split + 1, c21);
  rooms[next].corner2 = rooms[id].corner2;
  rooms[id].corner2   = cp(split - 1, c22);

  uint8_t door = c21 + rand() % (c22 - c21 + 1);

  for (uint8_t n = c21; n <= c22; ++n) {
    uint8_t templateId = gen_template(depth, door == n ? DOOR : WALL);
    if (!level[cp(split, n)])
      add_entity(templateId, cp(split, n));
  }

  return true;
}

bool in_room(uint8_t pos, uint8_t roomId) {
  room_t room = rooms[roomId];
  return pos % WIDTH >= room.corner1 % WIDTH - 1 &&
         pos / WIDTH >= room.corner1 / WIDTH - 1 &&
         pos % WIDTH <= room.corner2 % WIDTH + 1 &&
         pos / WIDTH <= room.corner2 / WIDTH + 1;
}

void discover_rooms(uint8_t pos) {
  for (uint8_t i = 0; i < MAX_ROOMS; ++i)
    if (in_room(pos, i))
      rooms[i].flags &= ~HIDDEN;
}

bool visible(uint8_t pos) {
  for (uint8_t i = 0; i < MAX_ROOMS; ++i)
    if (!(rooms[i].flags & HIDDEN) && in_room(pos, i))
      return true;
  return false;
}

uint8_t random_point_in_room(uint8_t roomId) {
  room_t room = rooms[roomId];
  uint8_t width  = room.corner2 % WIDTH - room.corner1 % WIDTH + 1;
  uint8_t height = room.corner2 / WIDTH - room.corner1 / WIDTH + 1;
  return room.corner1 + rand() % width
                      + rand() % height * WIDTH;
}

bool add_to_room(uint8_t roomId, uint8_t templateId) {
  uint8_t pos = random_point_in_room(roomId);
  if (!level[pos])
    return add_entity(templateId, pos);
  return false;
}

void first_level(uint8_t depth) {
  // add player
  uint8_t start_pos = 0;//rand() % (WIDTH * HEIGHT);
  add_entity(gen_template(depth, PLAYER), start_pos);

  // add stairs into dungeon
  uint8_t stairs = add_entity(
      gen_template(depth, FEATURE),
      start_pos + ((start_pos % WIDTH) ? -1 : 1));
  entities[stairs].hp = depth;

  // enter dungeon
  next_level(stairs);
}

bool next_level(entityId stairs) {
  entityId player = find_entity(PLAYER);
  entities[stairs].templateId++;
  uint8_t depth = entities[stairs].hp + 1;

  //  clear level except for stairs up and player
  for (int i = 0; i < WIDTH*HEIGHT; ++i)
    if (level[i] && level[i] != stairs && level[i] != player) {
      remove_entity(level[i]);
      level[i] = 0;
    }
  
  // initialize rooms
  for (int i = 0; i < MAX_ROOMS; ++i) {
    // rooms unused if corner1 > corner2
    rooms[i].corner1 = 1;
    rooms[i].corner2 = 0;
    rooms[i].flags  = 0;
  }

  // construct first room and iteratively split
  rooms[0].corner1 = 0;
  rooms[0].corner2 = (WIDTH - 1) + WIDTH * (HEIGHT - 1);
  uint8_t room_count = 1;
  while (split_room(rooms, depth))
    ++room_count;

  // determine entry room, hide all other rooms
  uint8_t entry = 0;
  for (int i = 0; i < MAX_ROOMS; ++i) {
    //rooms[i].flags &= ~HIDDEN;
    rooms[i].flags |= HIDDEN;
    if (in_room(entities[player].pos, i)) {
      entry = i;
      rooms[i].flags &= ~HIDDEN;
    }
  }

  // add stairs down
  uint8_t stairs_down = 0;
  while (!stairs_down) {
    uint8_t i = (entry + 1 + rand() % (room_count-1)) % room_count;
    uint8_t pos = random_point_in_room(i);
    if (rooms[i].corner2 % WIDTH - rooms[i].corner1 % WIDTH &&
        rooms[i].corner2 / WIDTH - rooms[i].corner1 / WIDTH ) {
      //&& !near_door(pos)) {
      stairs_down = add_entity(gen_template(depth, FEATURE), pos);
    }
  }
  entities[stairs_down].hp = depth;

  // fill rooms
  for (uint8_t i = 0; i < MAX_ROOMS; ++i) {
    if (!ROOM_EXISTS(i) || i == entry)
      continue;

    uint8_t monster_count = 0;
    while (rand() % 0xFF < MONSTER_SPAWN_RATE && monster_count++ < depth)
      add_to_room(i, gen_template(depth, MONSTER));

    uint8_t item_count = 0;
    while (rand() % 0xFF < ITEM_SPAWN_RATE && item_count++ < depth)
      add_to_room(i, gen_template(depth, ITEM));
  }

  return true;
}

#endif

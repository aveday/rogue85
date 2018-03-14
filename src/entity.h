#ifndef ENTITY_H
#define ENTITY_H

#include "config.h"
#include "graphics.h"
#include "sprites.h"

#define EMPTY 0
#define PLAYER 1
#define SKELETON 2
#define RAT 3
#define INVALID 255

#define TEMPLATE(id) templates[entities[id].templateId]

typedef uint8_t entityId;

typedef struct {
  uint8_t pos;
  uint8_t hp;
  uint8_t templateId;
} entity_t;

typedef struct {
  const sprite_t sprite;
  const uint8_t max_hp;
  void (*behaviour)(entityId);
} template_t;

entityId room[WIDTH*HEIGHT];
entity_t entities[MAX_ENTITIES];

void basic_ai(entityId id);
void player_control(entityId id);

const template_t templates[] PROGMEM = {
  {EMPTY_S,   ~0, NULL},
  {PLAYER_S,  10, player_control},
  {SKELETON_S, 2, basic_ai},
  {RAT_S,      1, basic_ai}
};

//TODO
bool is_targetable(entityId id) {
  return id > EMPTY && id < INVALID;
}

entityId add_entity(uint8_t templateId, uint8_t pos) {
  for (entityId id = 0; id < MAX_ENTITIES; ++id) {
    if (entities[id].templateId != INVALID) continue;

    entities[id].templateId = templateId;
    entities[id].pos = pos;
    entities[id].hp = pgm_read_byte_near(
        &templates[templateId].max_hp);

    room[pos] = id;
    return id;
  }
  return INVALID;
}

void remove_entity(entityId id) {
  room[entities[id].pos] = EMPTY;
  entities[id].templateId = INVALID;
  entities[id].pos = INVALID;
  entities[id].hp = INVALID;
}

entityId query_adjacent(entityId id, int8_t dx, int8_t dy) {
  uint8_t adj_pos = entities[id].pos + dx + WIDTH * dy;
  uint8_t adj_row = entities[id].pos / WIDTH + dy;
  return (adj_pos / WIDTH != adj_row) ? INVALID : room[adj_pos];
}

bool move(entityId id, int8_t dx, int8_t dy) {
  if (query_adjacent(id, dx, dy) != EMPTY)
    return false;
  room[entities[id].pos] = EMPTY;
  entities[id].pos += dx + WIDTH * dy;
  room[entities[id].pos] = id;
  return true;
}

bool attack(entityId id, int8_t dx, int8_t dy) {
  entityId target = query_adjacent(id, dx, dy);
  if (!is_targetable(target))
    return false;

  entities[target].hp--;
  if (!entities[target].hp)
    remove_entity(target);

  return true;
}

int8_t sign(int8_t a) {
  if (a > 0) return 1;
  if (a < 0) return -1;
  return 0;
}

void basic_ai(entityId id) {
  entityId player = 1; // FIXME

  // FOLLOW
  int8_t px = entities[player].pos % WIDTH;
  int8_t py = entities[player].pos / WIDTH;
  int8_t ex = entities[id].pos % WIDTH;
  int8_t ey = entities[id].pos / WIDTH;
  move(id, sign(px - ex), sign(py - ey));

  // ATTACK
  int8_t dx = px - entities[id].pos % WIDTH;
  int8_t dy = py - entities[id].pos / WIDTH;
  if ((abs(dx) == 1 && !dy) || (abs(dy) == 1 && !dx))
    attack(id, dx, dy);
}

bool handle_input(uint8_t input, entityId id) {
  int8_t dx = (input & LEFT) ? -1 : (input & RIGHT) ? 1 : 0;
  int8_t dy = (input & UP) ? -1 : (input & DOWN) ? 1 : 0;
  
  switch(input & 0b1111) {
    case 0: return move(id, dx, dy);
    case A: return attack(id, dx, dy);
    case B: return false;
    case X: return false;
    case Y: return false;
  }
  return false;
}

void player_control(entityId id) {
  uint8_t input = 0;
  do {
    while (get_input(LEFT_INPUT));            // await release
    while (!(input = get_input(LEFT_INPUT))); // await left input
    input <<= 4;                              // shift left input
    input += get_input(RIGHT_INPUT);          // get right state
  } while (!handle_input(input, id));
}

#endif

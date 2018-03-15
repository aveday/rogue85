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

// TEMPLATE FLAGS
#define VACANT  1 << 0
#define TARGET  1 << 1

#define TEMPLATE(id) templates[entities[id].templateId]
#define FIELD(type, id, field) (pgm_read_ ## type ## _near(&(TEMPLATE(id).field)))
#define FLAG(id, flag) (FIELD(byte, id, flags) & flag)

typedef uint8_t entityId;

typedef struct {
  uint8_t pos;
  uint8_t hp;
  uint8_t templateId;
} entity_t;

typedef struct {
  const sprite_t sprite;
  const uint8_t max_hp;
  const uint8_t flags;
  void (*behaviour)(entityId);
} template_t;

entityId room[WIDTH*HEIGHT];
entity_t entities[MAX_ENTITIES];

void basic_ai(entityId id);
void player_control(entityId id);

const template_t templates[] PROGMEM = {
  {EMPTY_S,   ~0,      VACANT, NULL},
  {BRICK_S,   ~0,           0, NULL},

  {PLAYER_S,  10,      TARGET, player_control},
  {SKELETON_S, 2,      TARGET, basic_ai},
  {RAT_S,      1,      TARGET, basic_ai},
};

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

bool in_bounds(entityId id, int8_t dx, int8_t dy) {
  uint8_t adj_pos = entities[id].pos + dx + WIDTH * dy;
  uint8_t adj_row = entities[id].pos / WIDTH + dy;
  return (adj_pos / WIDTH == adj_row && adj_row < HEIGHT);
}

entityId relative(entityId id, int8_t dx, int8_t dy) {
  return room[entities[id].pos + dx + WIDTH * dy];
}

bool move(entityId id, int8_t dx, int8_t dy) {
  // check if in bounds and vacant
  if (!in_bounds(id, dx, dy) ||
      !FLAG(relative(id, dx, dy), VACANT))
    return false;

  // move
  room[entities[id].pos] = EMPTY;
  entities[id].pos += dx + WIDTH * dy;
  room[entities[id].pos] = id;
  return true;
}

bool attack(entityId id, int8_t dx, int8_t dy) {
  if (!in_bounds(id, dx, dy))
      return false;

  entityId target = relative(id, dx, dy);
  if (!FLAG(target, TARGET))
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

#include "config.h"
#include "graphics.h"
#include "sprites.h"

#define EMPTY 0
#define PLAYER 1
#define SKELETON 2
#define RAT 3
#define INVALID 255

typedef uint8_t entityId;

typedef struct {
  uint8_t pos;
  uint8_t hp;
  uint8_t templateId;
} entity_t;

typedef struct {
  const sprite_t sprite;
  const uint8_t max_hp;
} template_t;

entityId room[WIDTH*HEIGHT];
entity_t entities[MAX_ENTITIES];

const template_t templates[] PROGMEM = {
  {EMPTY_S,   ~0},
  {PLAYER_S,   10},
  {SKELETON_S, 2},
  {RAT_S,      1}
};

void add_entity(uint8_t templateId, uint8_t pos) {
  for (entityId id = 0; id < MAX_ENTITIES; ++id) {
    if (entities[id].templateId != INVALID) continue;

    entities[id].templateId = templateId;
    entities[id].pos = pos;
    entities[id].hp = pgm_read_byte_near(
        &templates[templateId].max_hp);

    break;
  };
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

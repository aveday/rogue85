#include "config.h"
#include "graphics.h"

#define EMPTY 0
#define PLAYER 1
#define SKELETON 2
#define RAT 3
#define INVALID 255

typedef uint8_t entityId;

typedef struct {
  uint8_t pos;
  uint8_t templateId;
} entity_t;

typedef struct {
  const sprite_t* sprite;
} template_t;

entityId room[WIDTH*HEIGHT];
entity_t entities[MAX_ENTITIES];

const template_t templates[] = {
  {&empty_s},
  {&player_s},
  {&skeleton_s},
  {&rat_s}
};

void add_entity(uint8_t templateId, uint8_t pos) {
  for (entityId id = 0; id < MAX_ENTITIES; ++id) {
    if (entities[id].templateId != INVALID) continue;
    entities[id].templateId = templateId;
    entities[id].pos = pos;
    break;
  };
}

void move(entityId id, int8_t dx, int8_t dy) {
  room[entities[id].pos] = EMPTY;
  entities[id].pos += dx + WIDTH * dy;
  room[entities[id].pos] = id;
}

entityId query_adjacent(entityId id, int8_t dx, int8_t dy) {
  uint8_t adj_pos = entities[id].pos + dx + WIDTH * dy;
  uint8_t adj_row = entities[id].pos / WIDTH + dy;
  return (adj_pos / WIDTH != adj_row) ? INVALID : room[adj_pos];
}

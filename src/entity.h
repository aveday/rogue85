#ifndef ENTITY_H
#define ENTITY_H

#include "config.h"
#include "graphics.h"
#include "sprites.h"

// TEMPLATE FLAGS
#define PLAYER  1 << 1
#define MONSTER 1 << 2
#define TARGET  1 << 3
#define WALL    1 << 4
#define ITEM    1 << 5
#define DOOR    1 << 6

#define N_TEMPLATES (sizeof(templates) / sizeof(template_t))
#define TEMPLATE(id) templates[entities[id].templateId]
#define FIELD(type, id, field) (pgm_read_ ## type ## _near(&(TEMPLATE(id).field)))
#define FLAG(id, flag) (FIELD(byte, id, flags) & (flag))

#define SPAWN(spawnrate) (spawnrate & 0x0F)
#define DEPTH(spawnrate) ((spawnrate & 0xF0) >> 4)

typedef uint8_t entityId;

typedef struct {
  uint8_t pos;
  uint8_t hp;
  uint8_t templateId;
} entity_t;

typedef struct {
  const sprite_t sprite;
  const uint8_t spawnrate;
  const uint8_t max_hp;
  const uint8_t flags;
  void* behaviour;
} template_t;

entityId level[WIDTH*HEIGHT + INVENTORY];
entity_t entities[MAX_ENTITIES];
uint8_t selected = 0;

void basic_ai(entityId id);
void player_control(entityId id);
bool sword_attack(entityId id, int8_t dx, int8_t dy);

const template_t templates[] PROGMEM = {
  /*-ENVIRONMENT----|-------|-----|----------------|------------------|
  | sprite          | spawn |  hp |           type | action          */
  { BRICK_S         ,  0x18 ,   5 ,           WALL , NULL             },
  { DOOR_S          ,  0x18 ,   1 ,    DOOR|TARGET , NULL             },

  /*-MONSTERS-------|-------|-----|----------------|------------------|
  | sprite          | spawn |  hp |           type | action          */
  { RAT_S           ,  0x18 ,   1 , MONSTER|TARGET , basic_ai         },
  { ZOMBIE_S        ,  0x18 ,   2 , MONSTER|TARGET , basic_ai         },
  { SKELETON_S      ,  0x28 ,   3 , MONSTER|TARGET , basic_ai         },

  /*-PLAYER & ITEMS-|-------|-----|----------------|------------------|
  | sprite          | spawn |  hp |           type | action          */
  { PLAYER_S        ,  0x01 ,  20 ,  PLAYER|TARGET , player_control   },
  { SWORD_S         ,  0x18 ,  20 ,           ITEM , sword_attack     },   
  { WARRIOR_S       ,  0x00 ,  20 ,  PLAYER|TARGET , player_control   }, 
};

entityId find_entity(uint8_t flag) {
  uint8_t id = 0;
  while ( !entities[id].hp || !FLAG(id, flag) ) ++id;
  return id;
}

uint8_t gen_template(uint8_t depth, uint8_t flags) {
  uint16_t spawn_sum = 0;
  for (uint8_t templateId = 0; templateId < N_TEMPLATES; ++templateId) {
    uint8_t tFlags = pgm_read_byte_near(&templates[templateId].flags);
    uint8_t spawnrate = pgm_read_byte_near(&templates[templateId].spawnrate);
    if (!~(~flags | tFlags) && DEPTH(spawnrate) <= depth)
      spawn_sum += SPAWN(spawnrate);
  }
  uint16_t gen = rand() % spawn_sum;
  for (uint8_t templateId = 0; templateId < N_TEMPLATES; ++templateId) {
    uint8_t tFlags = pgm_read_byte_near(&templates[templateId].flags);
    uint8_t spawnrate = pgm_read_byte_near(&templates[templateId].spawnrate);
    if (!~(~flags | tFlags) && DEPTH(spawnrate) <= depth) {
      if (gen < SPAWN(spawnrate))
        return templateId;
      else
        gen -= SPAWN(spawnrate);
    }
  }
  return 0;
}

entityId add_entity(uint8_t templateId, uint8_t pos) {
  entityId id = 1;
  while (id < MAX_ENTITIES && entities[id].hp) ++id;
  if (id == MAX_ENTITIES)
    return 0;
  entities[id].templateId = templateId;
  entities[id].pos = pos;
  entities[id].hp = FIELD(byte, id, max_hp);
  return level[pos] = id;
}

void discover_rooms(uint8_t);
void remove_entity(entityId id) {
  level[entities[id].pos] = 0;
  entities[id].hp = 0;
  discover_rooms(entities[id].pos);
 
}

bool in_bounds(uint8_t pos, int8_t dx, int8_t dy) {
  uint8_t adj_pos = pos + dx + WIDTH * dy;
  uint8_t adj_row = pos / WIDTH + dy;
  return (adj_pos / WIDTH == adj_row && adj_row < HEIGHT);
}

entityId relative(entityId id, int8_t dx, int8_t dy) {
  return level[entities[id].pos + dx + WIDTH * dy];
}

bool move(entityId id, int8_t dx, int8_t dy) {
  // check if out of bounds or occupied
  if (!in_bounds(entities[id].pos, dx, dy) || relative(id, dx, dy))
    return false;

  // move
  level[entities[id].pos] = 0;
  entities[id].pos += dx + WIDTH * dy;
  level[entities[id].pos] = id;
  return true;
}

bool attack(entityId id, int8_t dx, int8_t dy) {
  if (!in_bounds(entities[id].pos, dx, dy))
      return false;

  entityId target = relative(id, dx, dy);
  if (!target || !FLAG(target, TARGET))
    return false;

  entities[target].hp--;
  if (!entities[target].hp)
    remove_entity(target);

  return true;
}

bool sword_attack(entityId id, int8_t dx, int8_t dy) {
  if (move(id, dx, dy)) {
    for (int8_t d = -1; d <= 1; d += 2) {
      attack(id, 0, d);
      attack(id, d, 0);
    }
    return true;
  }
  return attack(id, dx, dy);
}

bool use_item(entityId id, int8_t dx, int8_t dy) {
  uint8_t item = level[WIDTH*HEIGHT + selected];
  if (!item)
    return attack(id, dx, dy);

  bool (*action)(entityId, int8_t, int8_t) = FIELD(ptr, item, behaviour);
  return action(id, dx, dy);
}

bool select(entityId id, int8_t dx) {
  selected = (selected + INVENTORY + dx) % INVENTORY;

  entityId item = level[WIDTH*HEIGHT + selected];

  uint8_t newTemplate = entities[item].templateId;
  while (!(pgm_read_byte_near(&templates[newTemplate].flags) & PLAYER))
    ++newTemplate;

  entities[id].templateId = newTemplate;

  return true;
}

bool take(entityId id, int8_t dx, int8_t dy) {
  if (!in_bounds(entities[id].pos, dx, dy))
      return false;

  entityId target = relative(id, dx, dy);
  uint8_t pos = entities[id].pos + dx + WIDTH * dy;
  entityId equipped = level[WIDTH*HEIGHT + selected];

  if (FLAG(target, ITEM) && !equipped) {
    level[WIDTH*HEIGHT + selected] = target;
    level[pos] = 0;
  } else if (!target && equipped) {
    level[pos] = equipped;
    level[WIDTH*HEIGHT + selected] = 0;
  } else {
    return false;
  }

  select(id, 0);
  return true;
}

void basic_ai(entityId id) {
  int8_t sign(int8_t a) {
    if (a > 0) return 1;
    if (a < 0) return -1;
    return 0;
  }
  entityId player = find_entity(PLAYER);

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
    case A: return use_item(id, dx, dy);
    case B: return take(id, dx, dy);
    case X: return false;
    case Y: return select(id, dx);
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

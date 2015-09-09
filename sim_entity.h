//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_SIM_ENTITY_H
#define PHYSICA_SIM_ENTITY_H

#include "typedefs.h"
#include "physica_math.h"
#include "physica.h"

enum entity_type {
    PLAYER,
    TILE,
    WIZ_BUZZ
};

const u32 NO_FLAGS = 0;
const u32 REMOVED_FLAG = 1;
const u32 COLLIDES_FLAG = 2;
const u32 HAS_MASS_FLAG = 4;
const u32 REORIENTS_FLAG = 8;
const u32 JUST_CREATED_FLAG = 16;
const u32 DYING_FLAG = 32;
const u32 SIMULATED_FLAG = 64;
const u32 FIXED_FLAG = 128;
const u32 ROTATES_FLAG = 256;

struct aabb_t {
    v2 top_right, bottom_left;
};

struct sim_entity_t {
    u64 id;
    phy_body_t* body;
    f32 width;
    f32 height;
    u32 color;
};

struct sim_region_t {
    sim_entity_t* sim_entities;

    u32 sim_entity_count;
};

struct entity_low_t {
    sim_entity_t sim_entity;
};

#endif //PHYSICA_SIM_ENTITY_H

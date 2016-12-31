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
    BOGGER,
    BOGGER_BALL,
    TILE,
    WIZ_BUZZ,
    TURRET,
    TURRET_SHOT,
    SPIKES,
    SAVE_POINT,
    LILGUY,
};

#define UPDATE_FUNC(type) void update_##type(game_state_* game_state,\
                                             game_input_* game_input,\
                                             sim_entity_* entity,\
                                             f32 dt)

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

struct game_state_;

struct aabb_ {
    v2 top_right, bottom_left;
};

struct tile_info_ {
    i32 tex_coord_x;
    i32 tex_coord_y;
};

struct spikes_info_ {
    i32 direction;
};

struct bogger_state_ {
    f32 shoot_timer;
};

struct turret_state_ {
    f32 shoot_timer;
    v2 direction;
};

const u32 LILGUY_LEFT_FACING  = 0x01;
const u32 LILGUY_RUNNING      = 0x02;
const u32 LILGUY_MAYOR        = 0x04;

struct lilguy_state_ {
    u32 flags;
    i32 animation_index;
};

struct sim_entity_ {
    i64 id;
    phy_body_* body;
    entity_type type;

    union {
        tile_info_ tile_info;
        spikes_info_ spikes_info;
        bogger_state_ bogger_state;
        turret_state_ turret_state;
        lilguy_state_ lilguy_state;
        void* custom_state;
    };
};

struct sim_region_ {
    sim_entity_* sim_entities;

    u32 sim_entity_count;
};

struct entity_low_ {
    sim_entity_ sim_entity;
};

sim_entity_*
create_block_entity(game_state_* game_state,
                    entity_type type,
                    v2 position,
                    v2 diagonal,
                    f32 mass,
                    f32 orientation,
                    u32 flags);

sim_entity_*
create_fillet_block_entity(game_state_* game_state,
                    entity_type type,
                    v2 position,
                    v2 diagonal,
                    f32 fillet,
                    f32 mass,
                    f32 orientation,
                    u32 flags);

sim_entity_*
add_entity(game_state_* game_state);

void
remove_entity(game_state_* game_state, sim_entity_* entity);

#endif //PHYSICA_SIM_ENTITY_H

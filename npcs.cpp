#include "npcs.h"
#include "random.h"

const f32 LILGUY_WIDTH = 5.0f * VIRTUAL_PIXEL_SIZE;
const f32 LILGUY_HEIGHT = 10.0f * VIRTUAL_PIXEL_SIZE;
const f32 LILGUY_BLOCK_FILLET = 0.25f * LILGUY_WIDTH;
const f32 LILGUY_MASS = 2.5f;
const f32 LILGUY_MOVE_FACTOR = 15.0f;
const f32 LILGUY_RUN_SPEED = 1.5f;
const f32 LILGUY_Z = 0.1f;
const f32 LILGUY_AVERAGE_STATE_CHANGE_SECONDS = 1.0f;

animation_spec_t* get_animation(game_state_t* game_state, u32 flags) {
    animation_spec_t* spec = 0;

    switch (flags) {
        case LILGUY_MAYOR | LILGUY_RUNNING | LILGUY_LEFT_FACING: {
            spec = &game_state->animations.lilguy_mayor_running_left;
        } break;
        case LILGUY_MAYOR | LILGUY_RUNNING: {
            spec = &game_state->animations.lilguy_mayor_running_right;
        } break;
        case LILGUY_MAYOR | LILGUY_LEFT_FACING: {
            spec = &game_state->animations.lilguy_mayor_standing_left;
        } break;
        case LILGUY_MAYOR: {
            spec = &game_state->animations.lilguy_mayor_standing_right;
        } break;
        case LILGUY_RUNNING | LILGUY_LEFT_FACING: {
            spec = &game_state->animations.lilguy_running_left;
        } break;
        case LILGUY_RUNNING: {
            spec = &game_state->animations.lilguy_running_right;
        } break;
        case LILGUY_LEFT_FACING: {
            spec = &game_state->animations.lilguy_standing_left;
        } break;
        case 0: {
            spec = &game_state->animations.lilguy_standing_right;
        } break;
    }

    return spec;
}

sim_entity_t*
create_lilguy(game_state_t* game_state, v2 position, u32 flags) {

    sim_entity_t* entity = create_fillet_block_entity(game_state,
                                                      LILGUY,
                                                      position,
                                                      v2{LILGUY_WIDTH, LILGUY_HEIGHT},
                                                      LILGUY_BLOCK_FILLET,
                                                      LILGUY_MASS,
                                                      0.0f,
                                                      PHY_CHARACTER_FLAG);

    entity->body->inv_moment = 0.0f;
    
    b32 left_facing = false;
    b32 running = false;
    if (random_b32()) {
        flags |= LILGUY_LEFT_FACING;
        left_facing = true;
    }

    if (random_b32()) {
        flags |= LILGUY_RUNNING;
        running = true;
    }
    entity->lilguy_state.flags = flags;

    i32 animation_index =
        add_animation(&game_state->main_animation_group,
                      get_animation(game_state, flags),
                      LILGUY_Z);

    entity->lilguy_state.animation_index = animation_index;

    return entity;   
}

UPDATE_FUNC(LILGUY) {

    lilguy_state_t* state = &entity->lilguy_state;
    u32 previous_flags = state->flags;

    i32 chance = i32(LILGUY_AVERAGE_STATE_CHANGE_SECONDS / dt);
    if (random_i32(0, chance + 1) == 0) {
        if (random_b32()) {
            state->flags |= LILGUY_LEFT_FACING;
        } else {
            state->flags &= ~LILGUY_LEFT_FACING;
        }

        if (random_b32()) {
            state->flags |= LILGUY_RUNNING;
        } else {
            state->flags &= ~LILGUY_RUNNING;
        }
    }

    if (state->flags & LILGUY_LEFT_FACING) {
        ray_body_intersect_t ground = ray_cast(&game_state->physics_state,
                                               entity->body->position,
                                               normalize(v2 {-1.0f, -1.0f}),
                                               PHY_GROUND_FLAG);

        if (!ground.body || ground.depth > 1.0f) {
            state->flags &= ~LILGUY_RUNNING;
        }
    } else {
        ray_body_intersect_t ground = ray_cast(&game_state->physics_state,
                                               entity->body->position,
                                               normalize(v2 {1.0f, -1.0f}),
                                               PHY_GROUND_FLAG);

        if (!ground.body || ground.depth > 1.0f) {
            state->flags &= ~LILGUY_RUNNING;
        }
    }

    if (state->flags & LILGUY_RUNNING) {
        if (state->flags & LILGUY_LEFT_FACING) {

            f32 ddx = -LILGUY_MOVE_FACTOR * dt;
            entity->body->velocity.x =
                fmax(-LILGUY_RUN_SPEED, entity->body->velocity.x + ddx);
        } else {
            f32 ddx = LILGUY_MOVE_FACTOR * dt;
            entity->body->velocity.x =
                fmin(LILGUY_RUN_SPEED, entity->body->velocity.x + ddx);
        }
    } else {
        entity->body->velocity.x = 0.0f;
    }

    // update texture
    animation_t* animation = game_state->main_animation_group.animations
        .at(state->animation_index);

    animation->position = entity->body->position;
    animation->orientation = entity->body->orientation;

    if (state->flags != previous_flags) {
        reset_animation(animation, get_animation(game_state, state->flags));
    }
}

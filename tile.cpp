#include "sim_entity.h"

const v2 tile_diagonal = v2 {1.0f, 1.0f};
const f32 tile_z = 0.15f;
const i32 tile_texture_size = 64;
const f32 spikes_z = tile_z;
const i32 spikes_texture_size = tile_texture_size;

sim_entity_t*
create_tile(game_state_t* game_state, v2 position, tile_info_t info) {

    const f32 tile_mass = 100.0f;
    const f32 tile_orientation = 0.0f;

    sim_entity_t* tile = create_block_entity(game_state,
                                             TILE,
                                             position,
                                             tile_diagonal,
                                             tile_mass,
                                             tile_orientation,
                                             PHY_FIXED_FLAG | PHY_GROUND_FLAG);

    tile->body->inv_moment = 0.0f;
    tile->body->inv_mass = 0.0f;
    tile->tile_info = info;

    return tile;
}

UPDATE_FUNC(TILE) {
    phy_body_t* body = entity->body;

    rect_i source_rect;
    source_rect.min_x = entity->tile_info.tex_coord_x * tile_texture_size;
    source_rect.max_x = source_rect.min_x + tile_texture_size;
    source_rect.min_y = entity->tile_info.tex_coord_y * tile_texture_size;
    source_rect.max_y = source_rect.min_y + tile_texture_size;

    push_texture(&game_state->main_render_group,
                 body->position,
                 v2 {32.0f, 32.0f},
                 VIRTUAL_PIXEL_SIZE,
                 game_state->terrain_1,
                 source_rect,
                 rgba_t{0},
                 body->orientation,
                 tile_z);
}

sim_entity_t*
create_spikes(game_state_t* game_state, v2 position, i32 direction) {

    const f32 spikes_mass = 100.0f;
    const f32 spikes_orientation = 0.0f;

    v2 diagonal;
    v2 center;
    switch (direction) {
        case DIR_UP: {
            diagonal = v2 {1.0f, 0.5f};
            center = position - v2 {0.0f, 0.25f};
        } break;
        case DIR_DOWN: {
            diagonal = v2 {1.0f, 0.5f};
            center = position + v2 {0.0f, 0.25f};
        } break;
        case DIR_LEFT: {
            diagonal = v2 {0.5f, 1.0f};
            center = position - v2 {0.25f, 0.0f};
        } break;
        case DIR_RIGHT: {
            diagonal = v2 {0.5f, 1.0f};
            center = position + v2 {0.25f, 0.0f};
        } break;
    }

    sim_entity_t* spikes = create_block_entity(game_state,
                                               SPIKES,
                                               center,
                                               diagonal,
                                               spikes_mass,
                                               spikes_orientation,
                                               PHY_FIXED_FLAG);

    spikes->body->inv_moment = 0.0f;
    spikes->body->inv_mass = 0.0f;
    spikes->spikes_info.direction = direction;

    return spikes;
}

UPDATE_FUNC(SPIKES) {
    phy_body_t* body = entity->body;

    rect_i source_rect;
    source_rect.min_x = entity->spikes_info.direction * spikes_texture_size;
    source_rect.max_x = source_rect.min_x + spikes_texture_size;
    source_rect.min_y = 2 * spikes_texture_size;
    source_rect.max_y = source_rect.min_y + spikes_texture_size;

    v2 center;
    switch (entity->spikes_info.direction) {
        case DIR_UP: {
            center = body->position + v2 {0.0f, 0.25f};
        } break;
        case DIR_DOWN: {
            center = body->position - v2 {0.0f, 0.25f};
        } break;
        case DIR_LEFT: {
            center = body->position + v2 {0.25f, 0.0f};
        } break;
        case DIR_RIGHT: {
            center = body->position - v2 {0.25f, 0.0f};
        } break;
    }

    push_texture(&game_state->main_render_group,
                 center,
                 v2 {32.0f, 32.0f},
                 VIRTUAL_PIXEL_SIZE,
                 game_state->terrain_1,
                 source_rect,
                 rgba_t{0},
                 body->orientation,
                 spikes_z);


    entity_ties_t* collision = get_hash_item(&game_state->collision_map, entity->id);
    if (collision && collision->type == PLAYER) {
        kill_player(game_state);
    }
}

const v2 tile_diagonal = v2 {1.0f, 1.0f};
const f32 tile_z = 0.5f;
const i32 tile_texture_size = 32;

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
                                             PHY_FIXED_FLAG);

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
                 v2 {16.0f, 16.0f},
                 VIRTUAL_PIXEL_SIZE,
                 game_state->terrain_1,
                 source_rect,
                 body->orientation,
                 tile_z);
}
const v2 tile_diagonal = v2 {1.0f, 1.0f};
const f32 tile_z = 0.5f;

sim_entity_t*
create_tile(game_state_t* game_state, v2 position) {

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

    return tile;
}

UPDATE_FUNC(TILE) {
    phy_body_t* body = entity->body;

    push_rect(&game_state->main_render_group,
              to_rgb(0x8f8672),
              body->position,
              tile_diagonal,
              body->orientation,
              tile_z);
}
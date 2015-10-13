sim_entity_t*
create_tile(game_state_t* game_state, v2 position) {
	  i32 i = game_state->entities.push_unassigned();
    sim_entity_t* tile = game_state->entities.at(i);

    const v2 tile_diagonal = v2 {1.0f, 1.0f};
    const f32 tile_mass = 100.0f;

	  tile->body = phy_add_block(game_state->physics_arena,
                               position,
                               tile_diagonal,
                               tile_mass,
                               0.0f);

    tile->body->entity.id = tile->id = game_state->next_entity_id++;
    tile->body->entity.type = tile->type = TILE;

    tile->body->flags = PHY_FIXED_FLAG;
    tile->body->inv_moment = 0.0f;
    tile->body->inv_mass = 0.0f;

    push_rect(&game_state->main_render_group,
              color_t {1.0f,1.0f,1.0f},
              position,
              tile_diagonal,
              0.0f);

    tile->type = TILE;

    return tile;
}

UPDATE_FUNC(TILE) {
}
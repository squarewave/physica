struct tile_state_t {
    i32 tex_coord_x;
    i32 tex_coord_y;
};

sim_entity_t* create_tile(game_state_t* game_state, v2 position, tile_info_t info);

UPDATE_FUNC(TILE);
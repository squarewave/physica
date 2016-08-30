sim_entity_t* create_tile(game_state_t* game_state, v2 position, tile_info_t info);

UPDATE_FUNC(TILE);

sim_entity_t* create_spikes(game_state_t* game_state, v2 position, i32 direction);

UPDATE_FUNC(SPIKES);

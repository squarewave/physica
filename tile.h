sim_entity_* create_tile(game_state_* game_state, v2 position, tile_info_ info);

UPDATE_FUNC(TILE);

sim_entity_* create_spikes(game_state_* game_state, v2 position, i32 direction);

UPDATE_FUNC(SPIKES);

#ifndef PLAYER_H__
#define PLAYER_H__

struct player_state_t {
	b32 facing_right;
	i32 animation_index;
    v2 save_position;
    v2 save_gravity_normal;
    rotation_state_t save_rotation_state;
};

void kill_player(game_state_t* game_state, sim_entity_t* player);

sim_entity_t* create_player(game_state_t* game_state, v2 position);

UPDATE_FUNC(PLAYER);

sim_entity_t* create_save_point(game_state_t* game_state, v2 position);

UPDATE_FUNC(SAVE_POINT);

#endif /* end of include guard: PLAYER_H__ */

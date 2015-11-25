#ifndef PLAYER_H__
#define PLAYER_H__

enum player_animation_state_t {
    ANIM_JUMPING_RIGHT,
    ANIM_JUMPING_LEFT,
    ANIM_RUNNING_RIGHT,
    ANIM_RUNNING_LEFT,
    ANIM_STANDING_RIGHT,
    ANIM_STANDING_LEFT,
};

struct player_state_t {
	b32 facing_right;
	i32 animation_index;
    v2 save_position;
    v2 save_gravity_normal;
    u32 animation_state;
    rotation_state_t save_rotation_state;
};

void kill_player(game_state_t* game_state, sim_entity_t* player);

sim_entity_t* create_player(game_state_t* game_state, v2 position);

UPDATE_FUNC(PLAYER);

sim_entity_t* create_save_point(game_state_t* game_state, v2 position);

UPDATE_FUNC(SAVE_POINT);

#endif /* end of include guard: PLAYER_H__ */

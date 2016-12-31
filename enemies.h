#ifndef ENEMIES_H_
#define ENEMIES_H_

sim_entity_* create_turret(game_state_* game_state, v2 position, v2 direction);

UPDATE_FUNC(TURRET);

sim_entity_* create_turret_shot(game_state_* game_state, v2 position, v2 direction);

UPDATE_FUNC(TURRET_SHOT);

#endif

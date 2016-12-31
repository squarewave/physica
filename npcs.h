#ifndef NPCS_H_
#define NPCS_H_

#include "sim_entity.h"

sim_entity_* create_lilguy(game_state_* game_state,
                            v2 position, u32 flags);

UPDATE_FUNC(LILGUY);

#endif

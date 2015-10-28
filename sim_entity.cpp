#include "sim_entity.h"

sim_entity_t*
create_block_entity(game_state_t* game_state,
                    entity_type type,
                    v2 position,
                    v2 diagonal,
                    f32 mass,
                    f32 orientation,
                    u32 flags) {
    sim_entity_t* entity = game_state->entities.acquire();

	phy_body_t* body =  phy_add_block(game_state->physics_arena,
	                                  position,
	                                  diagonal,
	                                  mass,
	                                  orientation);

	body->flags = flags;
    body->position = position;
    body->entity.id = entity->id = game_state->next_entity_id++;
    body->entity.type = entity->type = type;

    entity->body = body;
    return entity;
}

sim_entity_t*
create_fillet_block_entity(game_state_t* game_state,
                           entity_type type,
                           v2 position,
                           v2 diagonal,
                           f32 fillet,
                           f32 mass,
                           f32 orientation,
                           u32 flags) {
    sim_entity_t* entity = game_state->entities.acquire();

	phy_body_t* body =  phy_add_fillet_block(game_state->physics_arena,
	                                         position,
	                                         diagonal,
	                                         fillet,
	                                         mass,
	                                         orientation);

	body->flags = flags;
    body->position = position;
    body->entity.id = entity->id = game_state->next_entity_id++;
    body->entity.type = entity->type = type;

    entity->body = body;
    return entity;
}

void
remove_entity(game_state_t* game_state, sim_entity_t* entity) {
	phy_remove_body(game_state->physics_arena,
	                entity->body);	
    game_state->entities.free(entity);
}
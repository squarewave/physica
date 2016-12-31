#include "sim_entity.h"

sim_entity_*
create_block_entity(game_state_* game_state,
                    entity_type type,
                    v2 position,
                    v2 diagonal,
                    f32 mass,
                    f32 orientation,
                    u32 flags) {
    sim_entity_* entity = add_entity(game_state);

	phy_body_* body = phy_add_block(&game_state->physics_state,
                                     position,
                                     diagonal,
                                     mass,
                                     orientation);

	body->flags = flags;
    body->position = position;
    body->entity.id = entity->id;
    body->entity.type = entity->type = type;

    entity->body = body;
    return entity;
}

sim_entity_*
create_fillet_block_entity(game_state_* game_state,
                           entity_type type,
                           v2 position,
                           v2 diagonal,
                           f32 fillet,
                           f32 mass,
                           f32 orientation,
                           u32 flags) {
    sim_entity_* entity = add_entity(game_state);

    assert_(fillet < diagonal.x / 2.0f);
    assert_(fillet < diagonal.y / 2.0f);

	phy_body_* body =  phy_add_fillet_block(&game_state->physics_state,
	                                         position,
	                                         diagonal,
	                                         fillet,
	                                         mass,
	                                         orientation);

	body->flags = flags;
    body->position = position;
    body->entity.id = entity->id;
    body->entity.type = entity->type = type;

    entity->body = body;
    return entity;
}

sim_entity_*
add_entity(game_state_* game_state) {
    sim_entity_* entity = game_state->entities.acquire();
    entity->id = game_state->next_entity_id++;
    set_hash_item(&game_state->entity_map, (u64)entity->id, entity);
    return entity;
}

void
remove_entity(game_state_* game_state, sim_entity_* entity) {
	phy_remove_body(&game_state->physics_state,
	                entity->body);
    remove_hash_item(&game_state->entity_map, (u64)entity->id);
    game_state->entities.free(entity);
}

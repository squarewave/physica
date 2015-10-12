sim_entity_t* create_player(game_state_t* game_state, v2 position) {

	const v2 player_diagonal = v2 {0.6f, 0.6f};
	const f32 player_block_fillet = 0.15f;
	const f32 player_mass = 10.0f;
	const f32 player_initial_orientation = 0.0f;

	i32 i = game_state->entities.push_unassigned();
    sim_entity_t* player = game_state->entities.at(i);

	phy_body_t* body =  phy_add_fillet_block(game_state->physics_arena,
	                                         position,
	                                         player_diagonal,
	                                         player_block_fillet, 
	                                         player_mass,
	                                         player_initial_orientation);

    player->body = body;

    i32 animation_index =
        add_animation(&game_state->main_render_group,
                      &game_state->main_animation_group,
                      &game_state->wiz_walking_right);

    player->body->position = position;
    player->type = PLAYER;
    player->body->inv_moment = 0.0f;
    player_state_t* player_state = PUSH_STRUCT(&game_state->world_arena,
                                               player_state_t);
    player_state->animation_index = animation_index;
    player->custom_state = (void*)player_state;

    return player;
}

void update_player(game_state_t* game_state,
                  game_input_t game_input,
                  sim_entity_t* entity,
                  f32 dt) {
	player_state_t* player = (player_state_t*)entity->custom_state;

    game_state->camera.center = game_state->camera.scaling *
    	game_state->camera.pixels_per_meter *
    	entity->body->position;

    const f32 player_move_factor = 80.0f;
    const f32 direction_deadzone = 0.1f;
    const f32 max_vel = 14.0f;
    const f32 jump_speed = 20.0f;
    const f32 jump_falloff = 7.0f;
    const f32 jump_velocity_threshold = 0.5f;
    const f32 jump_min_distance = 0.4f;
    const f32 jump_raycast_threshold = 0.5f;
    const v2 down = v2 {0,-1};

    // left/right movement
    if (game_input.joystick_l.position.x > direction_deadzone) {
        if (entity->body->velocity.x < 0.0f) {
            entity->body->velocity.x = 0.0f;
        }
        if (entity->body->velocity.x <= max_vel) {
            entity->body->velocity.x = entity->body->velocity.x +
                player_move_factor * dt * game_input.joystick_l.position.x;   
        }
    } else if (game_input.joystick_l.position.x < -direction_deadzone) {
        if (entity->body->velocity.x > 0.0f) {
            entity->body->velocity.x = 0.0f;
        }
        if (entity->body->velocity.x >= -max_vel) {
            entity->body->velocity.x = entity->body->velocity.x +
                player_move_factor * dt * game_input.joystick_l.position.x;   
        }
    } else {
        entity->body->velocity.x = 0.0f;
    }

    // jump
    if (game_input.button_a.ended_down) {
        if (entity->body->velocity.y < jump_velocity_threshold) {
            ray_body_intersect_t r =
                ray_cast_from_body(game_state->physics_arena,
                                   entity->body,
                                   jump_raycast_threshold,
                                   down);

            if (r.body && r.depth < jump_min_distance) {
                entity->body->velocity.y = jump_speed;
            }
        }
    } else {
        if (entity->body->velocity.y > jump_falloff &&
            game_input.button_a.transition_count > 0) {

            entity->body->velocity.y -= jump_falloff;
        }
    }

    // update texture
    game_state->main_animation_group.animations
    	.at(player->animation_index)->render_object->render_texture.center =
    	entity->body->position;
}
sim_entity_t* create_player(game_state_t* game_state, v2 position) {

	const v2 player_diagonal = v2 {0.6f, 0.6f};
	const f32 player_block_fillet = 0.2f;
	const f32 player_mass = 10.0f;
	const f32 player_initial_orientation = 0.0f;
    const f32 player_z_index = 2.0f;
    const u32 player_flags = 0;

    sim_entity_t* player = create_fillet_block_entity(game_state,
                                                      PLAYER,
                                                      position,
                                                      player_diagonal,
                                                      player_block_fillet,
                                                      player_mass,
                                                      player_initial_orientation,
                                                      player_flags);

    player->body->inv_moment = 0.0f;
    
    i32 animation_index =
        add_animation(&game_state->main_animation_group,
                      &game_state->wiz_walking_right,
                      player_z_index);

    player_state_t* player_state = PUSH_STRUCT(&game_state->world_arena,
                                               player_state_t);
    player_state->animation_index = animation_index;
    player_state->facing_right = true;

    player->custom_state = (void*)player_state;

    return player;
}

UPDATE_FUNC(PLAYER) {
	player_state_t* player = (player_state_t*)entity->custom_state;

    const f32 player_move_factor = 40.0f;
    const f32 direction_deadzone = 0.1f;
    const f32 max_vel = 7.0f;
    const f32 jump_speed = 16.0f;
    const f32 jump_falloff = 7.0f;
    const f32 jump_velocity_threshold = 0.5f;
    const f32 jump_min_distance = 0.4f;
    const f32 jump_raycast_threshold = 0.55f;
    const v2 down = v2 {0,-1};
    const f32 camera_move_factor = 0.4f;

    // v2 camera_target = game_state->camera.scaling *
    //     game_state->camera.pixels_per_meter *
    //     entity->body->position;
    // v2 d_camera = camera_target - game_state->camera.center;
    // game_state->camera.center = game_state->camera.center +
    //     (camera_move_factor * d_camera);

    game_state->camera.center = entity->body->position;

    b32 direction_changed = false;
    b32 moving = false;

    // left/right movement
    if (game_input.joystick_l.position.x > direction_deadzone) {
    	moving = true;
    	if (!player->facing_right) {
	    	player->facing_right = true;
	    	direction_changed = true;
    	}

        if (entity->body->velocity.x < 0.0f) {
            entity->body->velocity.x = 0.0f;
        }
        entity->body->velocity.x = fmin(max_vel, entity->body->velocity.x +
            player_move_factor * dt * game_input.joystick_l.position.x);

    } else if (game_input.joystick_l.position.x < -direction_deadzone) {
    	moving = true;
    	if (player->facing_right) {
	    	player->facing_right = false;
	    	direction_changed = true;
    	}

        if (entity->body->velocity.x > 0.0f) {
            entity->body->velocity.x = 0.0f;
        }
        entity->body->velocity.x = fmax(-max_vel, entity->body->velocity.x +
            player_move_factor * dt * game_input.joystick_l.position.x);   
    } else {
        entity->body->velocity.x = 0.0f;
    }

    ray_body_intersect_t r =
        ray_cast_from_body(game_state->physics_arena,
                           entity->body,
                           jump_raycast_threshold,
                           down);

    b32 is_supported = r.body && r.depth < jump_min_distance;

    // jump
    if (game_input.button_a.ended_down) {
        if (is_supported && entity->body->velocity.y < jump_velocity_threshold) {
            entity->body->velocity.y = jump_speed;
        }
    } else if (entity->body->velocity.y > jump_falloff && 
               game_input.button_a.transition_count > 0) {
        entity->body->velocity.y -= jump_falloff;
    }

    // update texture
    animation_t* animation = game_state->main_animation_group.animations
    	.at(player->animation_index);

    animation->position = entity->body->position;

	if (player->facing_right) {
		animation->speed = 1.0f;
		if (!is_supported) {
			animation->spec = &game_state->wiz_jumping_right;
		} else if (moving) {
			animation->spec = &game_state->wiz_walking_right;
		} else {
			animation->spec = &game_state->wiz_standing_right;
		}
	} else {
		animation->speed = 1.0f;
		if (!is_supported) {
			animation->spec = &game_state->wiz_jumping_left;
		} else if (moving) {
			animation->spec = &game_state->wiz_walking_left;
		} else {
			animation->spec = &game_state->wiz_standing_left;
		}
	}
}

const f32 player_height = vpixels_to_meters(18.0f);
const f32 player_width = vpixels_to_meters(8.0f);

sim_entity_t* create_player(game_state_t* game_state, v2 position) {

	const v2 player_diagonal = v2 {player_width, player_height};
	const f32 player_block_fillet = 0.25f * player_width;
	const f32 player_mass = 10.0f;
	const f32 player_initial_orientation = 0.0f;
    const f32 player_z_index = 0.2f;
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
                      &game_state->animations.martin_standing_right,
                      player_z_index);

    player_state_t* player_state = PUSH_STRUCT(&game_state->world_arena,
                                               player_state_t);
    player_state->animation_index = animation_index;
    player_state->facing_right = true;

    player->custom_state = (void*)player_state;

    game_state->player = player;

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
    const f32 jump_min_distance = (0.5f * player_height) + 0.1f;
    const f32 jump_raycast_threshold = player_width / 2;
    const f32 camera_move_factor = 0.4f;

    // v2 camera_target = game_state->camera.scaling *
    //     game_state->camera.pixels_per_meter *
    //     entity->body->position;
    // v2 d_camera = camera_target - game_state->camera.center;
    // game_state->camera.center = game_state->camera.center +
    //     (camera_move_factor * d_camera);

    f32 gravity_orientation = atanv(game_state->gravity_normal) + fPI_OVER_2;

    f32 sum_trigger_delta =
        game_input.analog_l_trigger.delta - game_input.analog_r_trigger.delta;

    gravity_orientation += sum_trigger_delta * fPI_OVER_2;
    game_state->gravity_normal = v2_from_theta(gravity_orientation - fPI_OVER_2);

    game_state->camera.center = entity->body->position;
    game_state->camera.orientation = gravity_orientation;

    b32 direction_changed = false;
    b32 moving = false;

    f32 virtual_dx = flt_cross(game_state->gravity_normal, entity->body->velocity);
    f32 virtual_dy = -dot(game_state->gravity_normal, entity->body->velocity);

    // left/right movement
    if (game_input.joystick_l.position.x > direction_deadzone) {
    	moving = true;
    	if (!player->facing_right) {
	    	player->facing_right = true;
	    	direction_changed = true;
    	}

        if (virtual_dx < 0.0f) {
            virtual_dx = 0.0f;
        }
        virtual_dx = fmin(max_vel, virtual_dx +
            player_move_factor * dt * game_input.joystick_l.position.x);

    } else if (game_input.joystick_l.position.x < -direction_deadzone) {
    	moving = true;
    	if (player->facing_right) {
	    	player->facing_right = false;
	    	direction_changed = true;
    	}

        if (virtual_dx > 0.0f) {
            virtual_dx = 0.0f;
        }
        virtual_dx = fmax(-max_vel, virtual_dx +
            player_move_factor * dt * game_input.joystick_l.position.x);   
    } else {
        virtual_dx = 0.0f;
    }

    ray_body_intersect_t r =
        ray_cast_from_body(&game_state->physics_state,
                           entity->body,
                           jump_raycast_threshold,
                           game_state->gravity_normal);

    b32 is_supported = r.body && r.depth < jump_min_distance;

    // jump
    if (game_input.button_a.ended_down) {
        if (is_supported && virtual_dy < jump_velocity_threshold) {
            virtual_dy = jump_speed;
        }
    } else if (virtual_dy > jump_falloff && 
               game_input.button_a.transition_count > 0) {
        virtual_dy -= jump_falloff;
    }

    // update texture
    animation_t* animation = game_state->main_animation_group.animations
    	.at(player->animation_index);

    animation->position = entity->body->position;
    animation->orientation = gravity_orientation;
    entity->body->orientation = gravity_orientation;
    phy_update_body(&game_state->physics_state, entity->body);

	if (player->facing_right) {
		animation->speed = 1.0f;
		if (!is_supported) {
			animation->spec = &game_state->animations.martin_jumping_right;
		} else if (moving) {
			animation->spec = &game_state->animations.martin_running_right;
		} else {
			animation->spec = &game_state->animations.martin_standing_right;
		}
	} else {
		animation->speed = 1.0f;
		if (!is_supported) {
			animation->spec = &game_state->animations.martin_jumping_left;
		} else if (moving) {
            animation->spec = &game_state->animations.martin_running_left;
		} else {
            animation->spec = &game_state->animations.martin_standing_left;
		}
	}

    entity->body->velocity = rotate(v2{virtual_dx, virtual_dy}, gravity_orientation);
}

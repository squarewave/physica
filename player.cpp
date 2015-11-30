const f32 player_height = vpixels_to_meters(18.0f);
const f32 player_width = vpixels_to_meters(8.0f);

sim_entity_t* create_player(game_state_t* game_state, v2 position) {

	const v2 player_diagonal = v2 {player_width, player_height};
	const f32 player_block_fillet = 0.25f * player_width;
	const f32 player_mass = 10.0f;
	const f32 player_initial_orientation = 0.0f;
    const f32 player_z_index = 0.1f;
    const u32 player_flags = PHY_CHARACTER_FLAG;

    sim_entity_t* player = create_fillet_block_entity(game_state,
                                                      PLAYER,
                                                      position,
                                                      player_diagonal,
                                                      player_block_fillet,
                                                      player_mass,
                                                      player_initial_orientation,
                                                      player_flags);

    player->body->inv_moment = 0.0f;
    player->body->gravity_normal = v2 {0.0f, -1.0f};
    
    i32 animation_index =
        add_animation(&game_state->main_animation_group,
                      &game_state->animations.may_standing_right,
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

    const f32 player_move_factor = 20.0f;
    const f32 direction_deadzone = 0.1f;
    const f32 max_vel = 3.5f;
    const f32 jump_speed = 7.0f;
    const f32 jump_falloff = 3.5f;
    const f32 jump_velocity_threshold = 0.25f;
    const f32 jump_min_distance = (0.5f * player_height) + 0.1f;
    const f32 jump_raycast_threshold = player_width / 2;
    const f32 camera_move_factor = 0.4f;

    f32 gravity_orientation = atanv(entity->body->gravity_normal) + fPI_OVER_2;

    f32 combined_l_r_trigger = game_input->analog_r_trigger.value -
        game_input->analog_l_trigger.value;

    if (game_state->rotation_state.target_direction ==
        game_state->rotation_state.current_direction) {
        if (combined_l_r_trigger < -0.75f) {
            if (!game_state->rotation_state.needs_reset) {
                game_state->rotation_state.needs_reset = true;
                game_state->rotation_state.target_direction--;
                if (game_state->rotation_state.target_direction == -1) {
                    game_state->rotation_state.target_direction = 3;
                }
                game_state->rotation_state.progress = 0.0f;
            }
        } else if (combined_l_r_trigger > 0.75f) {
            if (!game_state->rotation_state.needs_reset) {
                game_state->rotation_state.needs_reset = true;
                game_state->rotation_state.target_direction++;

                if (game_state->rotation_state.target_direction == 4) {
                    game_state->rotation_state.target_direction = 0;
                }
                game_state->rotation_state.progress = 0.0f;
            }
        } else if (fabs(combined_l_r_trigger) < 0.5f) {
            game_state->rotation_state.needs_reset = false;
        }
    }

    game_state->main_camera.center = entity->body->position;
    f32 camera_adjustment = -0.1f * fmin(fmax(combined_l_r_trigger, -0.5f), 0.5f);
    game_state->main_camera.orientation = gravity_orientation + camera_adjustment;

    b32 direction_changed = false;
    b32 moving = false;

    f32 old_virtual_dx = flt_cross(entity->body->gravity_normal, entity->body->velocity);
    f32 old_virtual_dy = -dot(entity->body->gravity_normal, entity->body->velocity);
    f32 virtual_dx = old_virtual_dx;
    f32 virtual_dy = old_virtual_dy;

    ray_body_intersect_t r =
        ray_cast_from_body(&game_state->physics_state,
                           entity->body,
                           jump_raycast_threshold,
                           entity->body->gravity_normal,
                           PHY_GROUND_FLAG);

    b32 is_supported = r.body && r.depth < jump_min_distance;

    // left/right movement
    if (game_input->joystick_l.position.x > direction_deadzone) {
    	moving = true;
    	if (!player->facing_right) {
	    	player->facing_right = true;
	    	direction_changed = true;
    	}

        f32 ddx = player_move_factor * dt * game_input->joystick_l.position.x;
        f32 test_dx = fmin(max_vel, virtual_dx + ddx);

        if (virtual_dx < 0.0f) {
            if (is_supported) {
                virtual_dx = 0.0f;
            } else {
                virtual_dx = fmin(max_vel, virtual_dx +
                    player_move_factor * dt * game_input->joystick_l.position.x);
            }
        }

        virtual_dx = fmax(test_dx, old_virtual_dx);   

    } else if (game_input->joystick_l.position.x < -direction_deadzone) {
    	moving = true;
    	if (player->facing_right) {
	    	player->facing_right = false;
	    	direction_changed = true;
    	}

        if (virtual_dx > 0.0f) {
            if (is_supported) {
                virtual_dx = 0.0f;
            } else {
                virtual_dx = fmax(-max_vel, virtual_dx +
                    player_move_factor * dt * game_input->joystick_l.position.x);
            }
        }

        virtual_dx = fmax(-max_vel, virtual_dx +
            player_move_factor * dt * game_input->joystick_l.position.x);
    } else if (is_supported) {
        virtual_dx = 0.0f;
    }

    // jump
    if (game_input->button_a.ended_down) {
        if (is_supported && virtual_dy < jump_velocity_threshold) {
            is_supported = false;
            virtual_dy = jump_speed;
        }
    } else if (virtual_dy > jump_falloff && 
               game_input->button_a.transition_count > 0) {
        virtual_dy -= jump_falloff;
    }

    // update texture
    animation_t* animation = game_state->main_animation_group.animations
    	.at(player->animation_index);

    animation->position = entity->body->position;
    animation->orientation = gravity_orientation;
    entity->body->orientation = gravity_orientation;
    phy_update_body(&game_state->physics_state, entity->body);

    u32 intended_animation_state = 0;
	if (player->facing_right) {
		if (!is_supported) {
            intended_animation_state = ANIM_JUMPING_RIGHT;
		} else if (moving) {
            intended_animation_state = ANIM_RUNNING_RIGHT;
		} else {
            intended_animation_state = ANIM_STANDING_RIGHT;
		}
	} else {
		if (!is_supported) {
            intended_animation_state = ANIM_JUMPING_LEFT;
		} else if (moving) {
            intended_animation_state = ANIM_RUNNING_LEFT;
		} else {
            intended_animation_state = ANIM_STANDING_LEFT;
		}
	}

    if (player->animation_state != intended_animation_state) {
        switch (intended_animation_state) {
            case ANIM_RUNNING_RIGHT: {
                reset_animation(animation, &game_state->animations.may_running_right);
            } break;
            case ANIM_STANDING_RIGHT: {
                reset_animation(animation, &game_state->animations.may_standing_right);
            } break;
            case ANIM_RUNNING_LEFT: {
                reset_animation(animation, &game_state->animations.may_running_left);
            } break;
            case ANIM_STANDING_LEFT: {
                reset_animation(animation, &game_state->animations.may_standing_left);
            } break;
            case ANIM_JUMPING_RIGHT: {
                if (player->animation_state == ANIM_JUMPING_LEFT) {
                    set_animation(animation, &game_state->animations.may_jumping_right);
                } else {
                    reset_animation(animation, &game_state->animations.may_jumping_right);
                }
            } break;
            case ANIM_JUMPING_LEFT: {
                if (player->animation_state == ANIM_JUMPING_RIGHT) {
                    set_animation(animation, &game_state->animations.may_jumping_left);
                } else {
                    reset_animation(animation, &game_state->animations.may_jumping_left);
                }
            } break;
        }
        player->animation_state = intended_animation_state;
    }


    entity_ties_t* collision = get_hash_item(&game_state->collision_map, entity->id);
    if (collision && collision->type == SAVE_POINT) {
        player->save_position = entity->body->position;
        player->save_gravity_normal = entity->body->gravity_normal;
        player->save_rotation_state = game_state->rotation_state;
    }

    entity->body->velocity = rotate(v2{virtual_dx, virtual_dy}, gravity_orientation);
}

void kill_player(game_state_t* game_state) {
    sim_entity_t* player = game_state->player;
    player_state_t* player_state = (player_state_t*)player->custom_state;
    player->body->position = player_state->save_position;
    player->body->gravity_normal = player_state->save_gravity_normal;
    game_state->rotation_state = player_state->save_rotation_state;

    phy_update_body(&game_state->physics_state, player->body);
}

const v2 save_point_diagonal = v2 {1.0f, 1.0f};

sim_entity_t*
create_save_point(game_state_t* game_state, v2 position) {
    const f32 save_point_mass = 1.0f;
    const f32 save_point_orientation = 0.0f;

    sim_entity_t* save_point = create_block_entity(game_state,
                                             SAVE_POINT,
                                             position,
                                             save_point_diagonal,
                                             save_point_mass,
                                             save_point_orientation,
                                             PHY_FIXED_FLAG | PHY_INCORPOREAL_FLAG);

    return save_point;
}

UPDATE_FUNC(SAVE_POINT) {
}

const f32 turret_width = 0.5f;
const v2 turret_diagonal = v2 {turret_width, turret_width};
const f32 turret_shot_width = 0.2f;
const v2 turret_shot_diagonal = v2 {turret_shot_width, turret_shot_width};

sim_entity_* create_turret(game_state_* game_state, v2 position, v2 direction) {
    const f32 turret_mass = 5.0f;
    const f32 turret_orientation = 0.0f;

    sim_entity_* turret = create_block_entity(game_state,
                                               TURRET,
                                               position,
                                               turret_diagonal,
                                               turret_mass,
                                               turret_orientation,
                                               PHY_FIXED_FLAG);

    turret->body->inv_mass = 0.0f;
    turret->body->inv_moment = 0.0f;

    turret->turret_state.direction = direction;

    return turret;
}

UPDATE_FUNC(TURRET) {
    // const f32 turret_speed = 3.0f;
    const f32 turret_shoot_delay = 2.0f;

    turret_state_* state = &entity->turret_state;
    state->shoot_timer += dt;

    // f32 gravity_orientation = atanv(game_state->gravity_normal) + fPI_OVER_2;

    if (state->shoot_timer > turret_shoot_delay) {
        state->shoot_timer = 0.0f;
        v2 start_position = entity->body->position +
            entity->turret_state.direction * (0.5f * (turret_width + turret_shot_width));
        create_turret_shot(game_state, start_position, entity->turret_state.direction);
    }

    push_rect(&game_state->main_render_group,
              color_ {0.67f, 0.54f, 0.23f},
              entity->body->position,
              turret_diagonal,
              entity->body->orientation,
              0.5f,
              0);
}

sim_entity_* create_turret_shot(game_state_* game_state, v2 position, v2 direction) {
    const f32 turret_shot_mass = 0.1f;
    const f32 turret_shot_orientation = 0.0f;
    const f32 turret_shot_speed = 10.0f;

    sim_entity_* shot = create_fillet_block_entity(game_state,
                                                    TURRET_SHOT,
                                                    position,
                                                    turret_shot_diagonal,
                                                    0.09f,
                                                    turret_shot_mass,
                                                    turret_shot_orientation,
                                                    PHY_WEIGHTLESS_FLAG);

    shot->body->velocity = turret_shot_speed * direction;
    return shot;
}

UPDATE_FUNC(TURRET_SHOT) {
    entity_ties_* collision = get_hash_item(&game_state->collision_map,
      entity->id);
    if (collision && collision->type != TURRET) {
        if (collision->type == PLAYER) {
            kill_player(game_state);
        }
        
        remove_entity(game_state, entity);
    } else {
        push_rect(&game_state->main_render_group,
                  color_ {1.0f, 0.23f, 0.54f},
                  entity->body->position,
                  turret_shot_diagonal,
                  entity->body->orientation,
                  0.5f,
                  0);        
    }
}

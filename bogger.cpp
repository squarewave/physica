const v2 bogger_diagonal = v2 {0.5f, 0.5f};
const v2 bogger_ball_diagonal = v2 {0.2f, 0.2f};

sim_entity_* create_bogger(game_state_* game_state, v2 position) {
    const f32 bogger_mass = 5.0f;
    const f32 bogger_orientation = 0.0f;

    sim_entity_* bogger = create_block_entity(game_state,
                                               BOGGER,
                                               position,
                                               bogger_diagonal,
                                               bogger_mass,
                                               bogger_orientation,
                                               0);

    return bogger;
}

UPDATE_FUNC(BOGGER) {
    const f32 bogger_speed = 3.0f;
    const f32 bogger_shoot_delay = 2.0f;

    bogger_state_* state = &entity->bogger_state;
    state->shoot_timer += dt;

    f32 gravity_orientation = atanv(game_state->gravity_normal) + fPI_OVER_2;

    f32 virtual_dx = flt_cross(game_state->gravity_normal, entity->body->velocity);
    f32 virtual_dy = -dot(game_state->gravity_normal, entity->body->velocity);

    v2 to_player = normalize(game_state->player->body->position - entity->body->position);
    ray_body_intersect_ cast = ray_cast(&game_state->physics_state,
                                         entity->body->position,
                                         to_player,
                                         0,
                                         entity->body);

    if (cast.body == game_state->player->body) {
        if (flt_cross(to_player, game_state->gravity_normal) < 0.0f) {
            virtual_dx = bogger_speed; 
        } else {
            virtual_dx = -bogger_speed; 
        }

        if (state->shoot_timer > bogger_shoot_delay) {
            state->shoot_timer = 0.0f;
            create_turret_shot(game_state, entity->body->position, to_player);
        }
    } else {
        virtual_dx = 0.0f;
    }


    entity->body->orientation = gravity_orientation;
    entity->body->velocity = rotate(v2{virtual_dx, virtual_dy}, gravity_orientation);

    push_rect(&game_state->main_render_group,
              color_ {0.67f, 0.54f, 0.23f},
              entity->body->position,
              bogger_diagonal,
              entity->body->orientation,
              0.5f,
              0);
}

sim_entity_* create_bogger_ball(game_state_* game_state, v2 position, v2 direction) {
    const f32 bogger_ball_mass = 0.1f;
    const f32 bogger_ball_orientation = 0.0f;
    const f32 bogger_ball_speed = 20.0f;

    sim_entity_* ball = create_fillet_block_entity(game_state,
                                                    BOGGER_BALL,
                                                    position,
                                                    bogger_ball_diagonal,
                                                    0.09f,
                                                    bogger_ball_mass,
                                                    bogger_ball_orientation,
                                                    PHY_WEIGHTLESS_FLAG);

    ball->body->velocity = bogger_ball_speed * direction;
    return ball;
}

UPDATE_FUNC(BOGGER_BALL) {
    entity_ties_* collision = get_hash_item(&game_state->collision_map, entity->id);
    if (collision && collision->type != BOGGER) {
        remove_entity(game_state, entity);
    } else {
        push_rect(&game_state->main_render_group,
                  color_ {1.0f, 0.23f, 0.54f},
                  entity->body->position,
                  bogger_ball_diagonal,
                  entity->body->orientation,
                  0.5f,
                  0);        
    }
}

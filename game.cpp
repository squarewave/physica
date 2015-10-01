//
// Created by doug on 4/16/15.
//

#include <cfloat>
#include "game.h"
#include "game_render.h"
#include "physica.h"

void
initialize_game_state(game_state_t* game_state) {
    i64 memory_index = 0;
    u8* memory_location = (u8*)game_state + sizeof(game_state_t);

    #define __MAKE_ARENA(arena, count) do {\
        arena.size = (count);\
        arena.base = memory_location;\
        memory_location += (count);\
    } while (0)

    __MAKE_ARENA(game_state->world_arena, 1024L * 1024L * 64L);
    __MAKE_ARENA(game_state->physics_arena, 1024L * 1024L * 64L);
    __MAKE_ARENA(game_state->render_arena, 1024L * 1024L * 64L);

    game_state->entity_capacity = 4000;

    game_state->initialized = true;
}

struct physics_state_t {
    f32 x, y;
};

sim_entity_t*
make_block(game_state_t *game_state, f32 width, f32 height, f32 mass) {
    assert(game_state->entity_count < game_state->entity_capacity);
    sim_entity_t* result = game_state->entities + game_state->entity_count;
    game_state->entity_count++;

    result->width = width;
    result->height = height;
    result->body = phy_add_body(game_state->physics_arena);

    phy_body_t* body = result->body;
    body->mass = mass;
    body->inv_mass = 1.0f / body->mass;
    body->moment = 1.0f/12.0f * body->mass * (width * width + height * height);
    body->inv_moment = 1.0f / body->moment;
    body->hulls = phy_add_hulls(game_state->physics_arena, 1);
    body->aabb_node_index = -1;

    phy_hull_t * hull = body->hulls.values;
    hull->mass = mass;
    hull->inv_mass = 1.0f / hull->mass;
    hull->moment = 1.0f;
    hull->inv_moment = 1.0f/12.0f *
            hull->mass *
            (width * width + height * height);

    hull->type = HULL_MESH;
    hull->points = phy_add_points(game_state->physics_arena, 4);

    hull->points.values[0] = {result->width / 2.0f, result->height / 2.0f};
    hull->points.values[1] = {-result->width / 2.0f, result->height / 2.0f};
    hull->points.values[2] = {-result->width / 2.0f, -result->height / 2.0f};
    hull->points.values[3] = {result->width / 2.0f, -result->height / 2.0f};
    return result;
}

sim_entity_t*
make_fillet_block(game_state_t *game_state, f32 width, f32 height, f32 fillet, f32 mass) {
    assert(game_state->entity_count < game_state->entity_capacity);
    sim_entity_t* result = game_state->entities + game_state->entity_count;
    game_state->entity_count++;

    result->width = width;
    result->height = height;
    result->body = phy_add_body(game_state->physics_arena);

    phy_body_t* body = result->body;
    body->mass = mass;
    body->inv_mass = 1.0f / body->mass;
    body->moment = 1.0f/12.0f * body->mass * (width * width + height * height);
    body->inv_moment = 1.0f / body->moment;
    body->hulls = phy_add_hulls(game_state->physics_arena, 1);
    body->aabb_node_index = -1;

    phy_hull_t * hull = body->hulls.values;
    hull->mass = mass;
    hull->inv_mass = 1.0f / hull->mass;
    hull->moment = 1.0f;
    hull->inv_moment = 1.0f/12.0f *
            hull->mass *
            (width * width + height * height);
    hull->type = HULL_FILLET_RECT;
    hull->width = width;
    hull->height = height;
    hull->fillet = fillet;

    return result;
}

void add_blocks(game_state_t *game_state) {

    const u32 tile_map_width = 36;
    const u32 tile_map_height = 36;
    u32 tile_map_data[tile_map_width * tile_map_height] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1,
        1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
        1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
        1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
        1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1,
        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1,
        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1,
        1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
        1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
        1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1,
        1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
        1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1,
        1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
        1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
        1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1,
        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1,
        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
        1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1,
        1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    };

    for (int y = 0; y < tile_map_height; ++y) {
        for (int x = 0; x < tile_map_width; ++x) {
            if (tile_map_data[y * tile_map_width + x]) {
                f32 tile_size = 1.0f;
                sim_entity_t* tile = make_block(game_state, tile_size, tile_size, 100.0f);
                tile->color = 0xffffffff;
                tile->body->position.x = (tile_size / 2) + (x * tile_size);
                tile->body->position.y = (tile_size / 2) + (-(y + 1) * tile_size);
                tile->body->flags = PHY_FIXED_FLAG;
                tile->body->inv_moment = 0.0f;
                tile->body->inv_mass = 0.0f;
                tile->type = TILE;
            }
        }
    }

    sim_entity_t* player = make_fillet_block(game_state, 0.6f, 0.6f, 0.15f, 10.0f);
    player->body->position = v2 {10.0f, -4.0f};
    player->color = 0xffffffff;
    player->type = PLAYER;
    player->body->inv_moment = 0.0f;

    sim_entity_t* block = make_fillet_block(game_state, 5.0f, 5.0f, 0.0001f, 5.0f);
    block->body->position = v2 {8.0f, 10.0f};
    block->color = 0xffaaaaaa;
    block->type = TILE;
    block->body->inv_moment = 0.0f;

    const i32 max_render_objects = 1024 * 1024;
    game_state->main_render_group.objects.count = 0;
    game_state->main_render_group.objects.capacity = max_render_objects;
    game_state->main_render_group.objects.values =
        PUSH_ARRAY(&game_state->render_arena, max_render_objects, render_object_t);
}

i32
meters_to_pixels(f32 meters) {
    return meters * 30.0f;
}

void
game_update_and_render(platform_services_t platform,
                       game_state_t* game_state, f64 dt,
                       video_buffer_description_t buffer_description,
                       game_input_t game_input) {

    TIMED_FUNC();

//    dt /= 20.0f;
    if (!game_state->initialized) {
        initialize_game_state(game_state);

        game_state->camera.center = v2 {0.0f, 0.0f};
        game_state->camera.to_top_left = v2 {
                START_WIDTH / 2, START_HEIGHT / 2};
        game_state->camera.scaling = 1.0f;

        phy_init(game_state->physics_arena);

        game_state->entities = (sim_entity_t*)game_state->world_arena.base;

        add_blocks(game_state);

        phy_set_gravity(game_state->physics_arena, v2 {0, -92.8});
    }


    phy_update(game_state->physics_arena, dt);

    camera_t camera = game_state->camera;
    m3x3 scale = get_scaling_matrix(camera.scaling * 30.0f);

    for (int i = 0; i < game_state->entity_count; ++i) {
        sim_entity_t* entity = &game_state->entities[i];

        if (entity->type == PLAYER) {
            camera.center = camera.scaling * 30.0f * entity->body->position;

            const f32 player_move_factor = 80.0f;
            const f32 direction_deadzone = 0.1f;
            const f32 max_vel = 14.0f;
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
            if (game_input.button_a.ended_down && entity->body->velocity.y < 0.5f) {
                ray_body_intersect_t r =
                    ray_cast_from_body(game_state->physics_arena, entity->body, 0.50f, v2 {0,-1});

                if (r.body && r.depth < 0.4f) {
                    entity->body->velocity.y = 29.0f;
                }
            }

        }
    }


    push_rect(&game_state->main_render_group,
              v3 {0.3f, 0.35f, 0.3f},
              v2{0,0},
              v2 {(f32)buffer_description.width, (f32)buffer_description.height},
              0.0f);

    for (int i = 0; i < game_state->entity_count; ++i) {
        sim_entity_t* entity = &game_state->entities[i];

        push_rect(&game_state->main_render_group,
                  to_rgb(entity->color),
                  entity->body->position,
                  v2 {entity->width, entity->height},
                  entity->body->orientation);
    }

    draw_render_group(platform, buffer_description, camera, &game_state->main_render_group);
}

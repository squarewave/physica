//
// Created by doug on 4/16/15.
//

#include <cfloat>
#include "game.h"
#include "game_render.h"
#include "physica.h"
#include "wiz_animation.h"

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

void initialize_render_arena(game_state_t* game_state) {
    const i32 max_render_objects = 1024 * 1024;
    game_state->main_render_group.objects.count = 0;
    game_state->main_render_group.objects.capacity = max_render_objects;
    game_state->main_render_group.objects.values =
        PUSH_ARRAY(&game_state->render_arena, max_render_objects, render_object_t);

    const i32 max_animations = 1024 * 6;
    game_state->main_animation_group.animations.count = 0;
    game_state->main_animation_group.animations.capacity = max_animations;
    game_state->main_animation_group.animations.values =
        PUSH_ARRAY(&game_state->render_arena, max_animations, animation_t);

    game_state->main_animation_group.free_list.count = 0;
    game_state->main_animation_group.free_list.capacity = max_animations;
    game_state->main_animation_group.free_list.values =
        PUSH_ARRAY(&game_state->render_arena, max_animations, i32);

    const i32 max_animation_frames = 1024 * 36;
    game_state->animation_frames.count = 0;
    game_state->animation_frames.capacity = max_animation_frames;
    game_state->animation_frames.values =
        PUSH_ARRAY(&game_state->render_arena, max_animation_frames, animation_frame_t);

    game_state->wiz_bmp = load_wiz_bmp();
    game_state->wiz_walking_right = wiz_walking_right(&game_state->animation_frames,
                                                      game_state->wiz_bmp);
}

struct physics_state_t {
    f32 x, y;
};

sim_entity_t*
make_block(game_state_t *game_state, f32 width, f32 height, f32 mass) {
    assert(game_state->entity_count < game_state->entity_capacity);
    sim_entity_t* result = game_state->entities + game_state->entity_count;
    game_state->entity_count++;

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

    hull->points.values[0] = {width / 2.0f, height / 2.0f};
    hull->points.values[1] = {-width / 2.0f, height / 2.0f};
    hull->points.values[2] = {-width / 2.0f, -height / 2.0f};
    hull->points.values[3] = {width / 2.0f, -height / 2.0f};
    return result;
}

sim_entity_t*
make_fillet_block(game_state_t *game_state, f32 width, f32 height, f32 fillet, f32 mass) {
    assert(game_state->entity_count < game_state->entity_capacity);
    sim_entity_t* result = game_state->entities + game_state->entity_count;
    game_state->entity_count++;

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

    const char* tile_map =
    "-------------------------------------------------"
    "#################################################"
    "#         #####################           #######" 
    "#         ########   #                    #######" 
    "#         ########   #                    #######" 
    "#     x                                    ######" 
    "#######                        #########  #######" 
    "####################   #################  #######" 
    "####################   ##################  ######" 
    "#########################################  ######" 
    "################################   #####  #######" 
    "################################      ##    #####" 
    "#########################               #   #####" 
    "################################           ######" 
    "###################################        ######" 
    "######################################    #######" 
    "#######################################   #######" 
    "#######################################   #######" 
    "#################################################" 
    "#################################################" 
    "#################################################" 
    "#################################################" 
    "#################################################";

    i32 tile_map_width = i32(strchr(tile_map, '#') - tile_map);
    i32 tile_map_height = strlen(tile_map) / tile_map_width - 1;


    tile_map = tile_map + tile_map_width;

    for (int y = 0; y < tile_map_height; ++y) {
        for (int x = 0; x < tile_map_width; ++x) {
            f32 tile_size = 1.0f;
            v2 position = v2 {
                (tile_size / 2) + (x * tile_size),
                (tile_size / 2) + (-(y + 1) * tile_size)
            };

            if (tile_map[y * tile_map_width + x] == '#') {
                sim_entity_t* tile = make_block(game_state, tile_size, tile_size, 100.0f);


                tile->render_object = push_rect(&game_state->main_render_group,
                                                color_t {1.0f,1.0f,1.0f},
                                                position,
                                                v2 {tile_size, tile_size},
                                                0.0f);

                tile->body->position = position;
                tile->body->flags = PHY_FIXED_FLAG;
                tile->body->inv_moment = 0.0f;
                tile->body->inv_mass = 0.0f;
                tile->type = TILE;
            } else if (tile_map[y * tile_map_width + x] == 'x') {
                sim_entity_t* player = make_fillet_block(game_state, 0.6f, 0.6f, 0.15f, 10.0f);

                i32 index =
                    add_animation(&game_state->main_render_group,
                                  &game_state->main_animation_group,
                                  &game_state->wiz_walking_right);

                player->render_object = game_state->main_animation_group
                                        .animations.at(index)->render_object;
                player->body->position = position;
                player->type = PLAYER;
                player->body->inv_moment = 0.0f;           
            }
        }
    }
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


    if (!game_state->initialized) {
        initialize_game_state(game_state);
        initialize_render_arena(game_state);

        game_state->camera.center = v2 {0.0f, 0.0f};
        game_state->camera.to_top_left = v2 {
                START_WIDTH / 2, START_HEIGHT / 2};
        game_state->camera.scaling = 2.0f;

        phy_init(game_state->physics_arena);

        game_state->entities = (sim_entity_t*)game_state->world_arena.base;

        push_rect(&game_state->main_render_group,
                  v3 {0.3f, 0.35f, 0.3f},
                  v2{0,0},
                  v2 {(f32)buffer_description.width, (f32)buffer_description.height},
                  0.0f);



        add_blocks(game_state);

        phy_set_gravity(game_state->physics_arena, v2 {0, -73.8f});
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
            if (game_input.button_a.ended_down) {
                if (entity->body->velocity.y < 0.5f) {
                    ray_body_intersect_t r =
                        ray_cast_from_body(game_state->physics_arena, entity->body, 0.50f, v2 {0,-1});

                    if (r.body && r.depth < 0.4f) {
                        game_state->input_memo.flags |= INPUT_FLAG_JUMP_STARTED;
                        entity->body->velocity.y = 20.0f;
                    }
                }
            } else {
                if (entity->body->velocity.y > 7.0f &&
                    game_input.button_a.transition_count > 0) {
                    entity->body->velocity.y -= 7.0f;
                }
            }

            entity->render_object->render_texture.center = entity->body->position;
        }
    }

    update_animations(&game_state->main_animation_group, dt);

    draw_render_group(platform, buffer_description, camera, &game_state->main_render_group);
}

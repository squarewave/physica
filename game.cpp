//
// Created by doug on 4/16/15.
//

#include <cfloat>
#include "game.h"
#include "game_render.h"
#include "physica.h"
#include "player.h"
#include "tile.h"
#include "wiz_animation.h"
#include "background.h"
#include "animations.h"
#include "random.h"
#include "bogger.h"

void
initialize_render_arena(game_state_t* game_state) {
    const i32 max_render_objects = 20000;
    game_state->main_render_group.objects.count = 0;
    game_state->main_render_group.objects.capacity = max_render_objects;
    game_state->main_render_group.objects.values =
        PUSH_ARRAY(&game_state->render_arena, max_render_objects, render_object_t);

    const i32 max_background_objects = 60000;
    game_state->background_render_group.objects.count = 0;
    game_state->background_render_group.objects.capacity = max_render_objects;
    game_state->background_render_group.objects.values =
        PUSH_ARRAY(&game_state->render_arena, max_background_objects, render_object_t);

    const i32 max_animations = 1024 * 6;
    game_state->main_animation_group.animations.count = 0;
    game_state->main_animation_group.animations.capacity = max_animations;
    game_state->main_animation_group.animations.values =
        PUSH_ARRAY(&game_state->render_arena, max_animations, animation_t);

    game_state->main_animation_group.freed.count = 0;
    game_state->main_animation_group.freed.capacity = max_animations;
    game_state->main_animation_group.freed.values =
        PUSH_ARRAY(&game_state->render_arena, max_animations, i32);

    const i32 max_animation_frames = 1024 * 36;
    game_state->animation_frames.count = 0;
    game_state->animation_frames.capacity = max_animation_frames;
    game_state->animation_frames.values =
        PUSH_ARRAY(&game_state->render_arena, max_animation_frames, animation_frame_t);

    game_state->wiz_bmp = load_wiz_bmp();
    game_state->wiz_walking_right = wiz_walking_right(&game_state->animation_frames,
                                                      game_state->wiz_bmp);
    game_state->wiz_walking_left = wiz_walking_left(&game_state->animation_frames,
                                                    game_state->wiz_bmp);
    game_state->wiz_standing_right = wiz_standing_right(&game_state->animation_frames,
                                                        game_state->wiz_bmp);
    game_state->wiz_standing_left = wiz_standing_left(&game_state->animation_frames,
                                                      game_state->wiz_bmp);
    game_state->wiz_jumping_right = wiz_jumping_right(&game_state->animation_frames,
                                                      game_state->wiz_bmp);
    game_state->wiz_jumping_left = wiz_jumping_left(&game_state->animation_frames,
                                                    game_state->wiz_bmp);

    game_state->animations = load_animations(&game_state->render_arena);

    game_state->terrain_1 = load_bmp("assets/terrain_1.bmp", 1);
}

void
setup_frame_buffer(game_state_t* game_state, video_buffer_description_t buffer) {
    f32 width = (f32)buffer.width;
    f32 height = (f32)buffer.height;
    i32 size = 2 * uceil(sqrt(width * width + height * height));

    glGenFramebuffers(1, &game_state->frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, game_state->frame_buffer);

    glGenTextures(1, &game_state->color_buffer);
    glBindTexture(GL_TEXTURE_2D, game_state->color_buffer);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 i32(width * 2),
                 i32(height * 2),
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_INT_8_8_8_8,
                 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           game_state->color_buffer,
                           0);
}

void
setup_world(game_state_t *game_state) {

    const char* tile_map =
    "-------------------------------------------------"
    "#################################################"
    "#         #######################################" 
    "#   p     ########   ############################" 
    "#   p     ########   ######     #           #####" 
    "      s              ##########       ###       #" 
    "#######            b                ####  ##### #" 
    "############################ ###########  ##### #" 
    "####                              #######  #### #" 
    "####                            #########  #### #" 
    "#######           #                #####  ##### #" 
    "#################           b               ### #" 
    "####        ########################### #   ### #" 
    "###        ###                             #### #" 
    "#####                  ######################## #" 
    "##############                            ##### #" 
    "###########        ####################   ##### #" 
    "#########        ###      #############         #" 
    "#################### x #                    #####" 
    "###          ########################     #######" 
    "##########     #       ###     ####     #########" 
    "#############      ###              #     #######" 
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

            switch(tile_map[y * tile_map_width + x]) {
                case '#': {
                    tile_info_t info;
                    info.tex_coord_x = random_i32(0, 8);
                    info.tex_coord_y = 0;
                    if (y && (tile_map[(y-1) * tile_map_width + x] == '#')) {
                        info.tex_coord_y = 1;
                    }
                    create_tile(game_state, position, info);
                } break;
                case 's': {
                    create_player(game_state, position);
                } break;
                case 'b': {
                    create_bogger(game_state, position);
                } break;
                case 'p': {
                    const v2 tile_diagonal = v2 {1.0f, 1.0f};
                    const f32 tile_mass = 10.0f;

                    sim_entity_t* tile = create_fillet_block_entity(game_state,
                                               TILE,
                                               position,
                                               tile_diagonal,
                                               0.15f,
                                               tile_mass,
                                               0.3f,
                                               0);
                } break;
            }
        }
    }

    game_state->gravity_normal = v2 {0.0f, -1.0f};
    game_state->gravity_magnitude = 78.0f;
}

void
initialize_game_state(game_state_t* game_state, video_buffer_description_t buffer) {
    i64 memory_index = 0;
    u8* memory_location = (u8*)game_state + sizeof(game_state_t);

    #define __MAKE_ARENA(arena, count) do {\
        arena.size = (count);\
        arena.base = memory_location;\
        memory_location += (count);\
    } while (0)

    __MAKE_ARENA(game_state->world_arena, 1024L * 1024L * 64L);
    __MAKE_ARENA(game_state->render_arena, 1024L * 1024L * 64L);

    initialize_render_arena(game_state);

    f32 camera_height_meters =
        (buffer.height /
        (f32)uround((buffer.height / TARGET_VIEW_HEIGHT_IN_METERS) * VIRTUAL_PIXEL_SIZE)) *
        VIRTUAL_PIXEL_SIZE;
    game_state->camera.center = v2 {0.0f, 0.0f};
    f32 aspect_ratio = (f32)buffer.width / (f32)buffer.height;
    game_state->camera.to_top_left = v2 {
        camera_height_meters * 0.5f * aspect_ratio,
        camera_height_meters * 0.5f
    };
    game_state->camera.orientation = 0.0f;

    game_state->background_camera.to_top_left = v2 {
        camera_height_meters * 0.5f * aspect_ratio,
        camera_height_meters * 0.5f
    };

    game_state->physics_state = phy_init(&game_state->world_arena);

    const i32 entity_capacity = 4000;
    game_state->entities.allocate(&game_state->world_arena, entity_capacity);
    game_state->next_entity_id = 1L;

    const i32 collision_capacity = 2000;
    game_state->collision_map.pairs.values = PUSH_ARRAY(&game_state->world_arena,
                                                        collision_capacity,
                                                        hashpair<entity_ties_t>);
    game_state->collision_map.pairs.count = collision_capacity;

    setup_world(game_state);


    create_background(game_state, &game_state->background);

    setup_frame_buffer(game_state, buffer);

    game_state->initialized = true;
}

void
game_update_and_render(platform_services_t platform,
                       game_state_t* game_state, f64 dt,
                       video_buffer_description_t buffer,
                       game_input_t game_input) {

    TIMED_FUNC();

    if (!game_state->initialized) {
        initialize_game_state(game_state, buffer);
    }

    clear_hashmap(&game_state->collision_map);

    phy_set_gravity(&game_state->physics_state, 
                    game_state->gravity_magnitude * game_state->gravity_normal);

    phy_update(&game_state->physics_state, &game_state->collision_map, dt);

    for (int i = 0; i < game_state->entities.size; ++i) {
        sim_entity_t* entity = game_state->entities.try_get(i);

        if (!entity) {
            continue;
        }

        #define __UPDATE_CASE(type) case type: {\
            update_##type(game_state, game_input, entity, dt);\
        } break

        switch (entity->type) {
            __UPDATE_CASE(PLAYER);
            __UPDATE_CASE(TILE);
            __UPDATE_CASE(BOGGER);
            __UPDATE_CASE(BOGGER_BALL);
        }
    }

    if (game_input.mouse.left_click.transition_count &&
        game_input.mouse.left_click.ended_down) {
        m3x3 inverse_view_transform = get_inverse_view_transform_3x3(game_state->camera);
        v2 world_position = inverse_view_transform * game_input.mouse.normalized_position;
        game_state->debug_state.selected = pick_body(&game_state->physics_state,
                                                     world_position);
    }

    if (game_state->debug_state.selected) {
        push_circle(&game_state->main_render_group,
                    color_t {0.4f, 1.0f, 0.4f},
                    game_state->debug_state.selected->position,
                    2.0f * VIRTUAL_PIXEL_SIZE,
                    0.0f,
                    0);
    }

    debug_draw_physics(game_state);

    game_state->background_camera.center = PARALLAX_COEFFICIENT *
        game_state->camera.center;
    game_state->background_camera.orientation = game_state->camera.orientation; 

    update_background(&game_state->background,
                      &game_state->background_render_group,
                      game_state->background_camera,
                      dt);

    update_animations(&game_state->main_animation_group,
                      &game_state->main_render_group,
                      dt);

    glBindFramebuffer(GL_FRAMEBUFFER, game_state->frame_buffer);
    glViewport(0,0,buffer.width * 2, buffer.height * 2);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_render_group(platform,
                      buffer,
                      game_state->background_camera,
                      &game_state->background_render_group);

    draw_render_group(platform,
                      buffer,
                      game_state->camera,
                      &game_state->main_render_group);

    clear_render_group(&game_state->background_render_group);
    clear_render_group(&game_state->main_render_group);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0,0,buffer.width, buffer.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    present_frame_buffer(platform, game_state->color_buffer);
}

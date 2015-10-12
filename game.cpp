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

void
initialize_render_arena(game_state_t* game_state) {
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

void
setup_world(game_state_t *game_state) {

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

            switch(tile_map[y * tile_map_width + x]) {
                case '#': {
                    create_tile(game_state, position);
                } break;
                case 'x': {
                    create_player(game_state, position);
                } break;
            }
        }
    }
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
    __MAKE_ARENA(game_state->physics_arena, 1024L * 1024L * 64L);
    __MAKE_ARENA(game_state->render_arena, 1024L * 1024L * 64L);

    initialize_render_arena(game_state);

    game_state->camera.center = v2 {0.0f, 0.0f};
    game_state->camera.to_top_left = v2 {
            START_WIDTH / 2, START_HEIGHT / 2};
    game_state->camera.scaling = 2.0f;
    game_state->camera.pixels_per_meter = 30.0f;

    phy_init(game_state->physics_arena);

    const i32 entity_capacity = 4000;
    game_state->entities.values = PUSH_ARRAY(&game_state->world_arena,
                                             entity_capacity,
                                             sim_entity_t);
    game_state->entities.capacity = entity_capacity;

    push_rect(&game_state->main_render_group,
              v3 {0.3f, 0.35f, 0.3f},
              v2{0,0},
              v2 {(f32)buffer.width, (f32)buffer.height},
              0.0f);

    setup_world(game_state);

    phy_set_gravity(game_state->physics_arena, v2 {0, -73.8f});

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

    phy_update(game_state->physics_arena, dt);

    for (int i = 0; i < game_state->entities.count; ++i) {
        sim_entity_t* entity = game_state->entities.at(i);

        switch (entity->type) {
            case PLAYER: {
                update_player(game_state, game_input, entity, dt);
            } break;
        }
    }

    update_animations(&game_state->main_animation_group, dt);

    draw_render_group(platform,
                      buffer,
                      game_state->camera,
                      &game_state->main_render_group);
}

//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_GAME_H
#define PHYSICA_GAME_H

#include "typedefs.h"
#include "game_debug.h"
#include "game_render.h"
#include "physica_math.h"
#include "sim_entity.h"
#include "hashmap.h"
#include "animation.h"
#include "background.h"

const u32 FRAME_RATE = 30;
const u32 START_WIDTH = 960;
const u32 START_HEIGHT = 540;
const u32 LEFT_OFFSET = 0;
const u32 AUDIO_SAMPLE_RATE = 44100; // desired rate - CD quality
const u32 PERIOD_SIZE = AUDIO_SAMPLE_RATE / FRAME_RATE;
const u32 SOUND_BUFFER_SIZE = PERIOD_SIZE * 8;
const u32 VIDEO_STATE_SIZE = 1024LL * 1024LL;
const u32 PIXEL_SIZE = 2;
const f32 PIXELS_PER_METER = 30.0f;

struct platform_read_entire_file_result_t {
    u8* contents;
    u32 content_size;
};

struct spatial_partition_t {
    u32 capacity;
    u32* primary_entities;
    u32 primary_entity_count;
    u32* secondary_entities;
    u32 secondary_entity_count;
};

const u32 INPUT_FLAG_JUMP_STARTED = 1;

struct input_memo_t {
    u32 flags;

    f32 jump_elapsed;
};

struct game_state_t {
    u32 initialized;

    i64 next_entity_id;
    iterable_pool<sim_entity_t> entities;
    hashmap<entity_ties_t> collision_map;

    f32 spatial_partition_width;
    u32 spatial_partition_grid_width;
    spatial_partition_t* spatial_partitions;
    u32 spatial_partition_count;

    camera_t camera;
    camera_t background_camera;
    render_group_t main_render_group;
    render_group_t background_render_group;

    memory_arena_t world_arena;
    phy_memory_t physics_arena;
    memory_arena_t render_arena;

    tex2 main_panel;

    input_memo_t input_memo;

    vec<animation_frame_t> animation_frames;
    animation_group_t main_animation_group;

    tex2 wiz_bmp;

    animation_spec_t wiz_walking_right;
    animation_spec_t wiz_walking_left;
    animation_spec_t wiz_standing_right;
    animation_spec_t wiz_standing_left;
    animation_spec_t wiz_jumping_right;
    animation_spec_t wiz_jumping_left;

    animation_spec_t wiz_buzz;

    background_t background;
};

struct button_input_t {
    u32 transition_count;
    b32 ended_down;
};

struct analog_input_t {
    f32 delta, value;
};

struct joystick_input_t {
    v2 delta, position;
};

struct game_input_t {
    button_input_t button_a, button_b, button_x, button_y;
    button_input_t button_l_bumper, button_r_bumper;
    button_input_t button_l_stick, button_r_stick;
    analog_input_t analog_l_trigger, analog_r_trigger;
    joystick_input_t joystick_l, joystick_r;
    button_input_t up, down, right, left, lshift, rshift;
};

#pragma pack(push, 1)
struct bitmap_header_t {
    u16 file_type;
    u32 file_size;
    u16 reserved_1;
    u16 reserved_2;
    u32 bitmap_offset;
    u32 size;
    i32 width;
    i32 height;
    u16 planes;
    u16 bits_per_pixel;
    u32 compression;
    u32 size_of_bitmap;
    i32 horz_resolution;
    i32 vert_resolution;
    u32 colors_used;
    u32 colors_important;
    u32 red_mask;
    u32 green_mask;
    u32 blue_mask;
};
#pragma pack(pop)

struct platform_services_t;  

void game_update_and_render(platform_services_t platform,
                            game_state_t* game_state, f64 dt,
                            video_buffer_description_t buffer_description,
                            game_input_t game_input);

platform_read_entire_file_result_t platform_read_entire_file(const char * filename);

void platform_free_file_memory(void* memory);

#endif //PHYSICA_GAME_H

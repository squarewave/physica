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

const u32 FRAME_RATE = 60;
const u32 START_WIDTH = 960;
const u32 START_HEIGHT = 540;
const u32 LEFT_OFFSET = 0;
const u32 AUDIO_SAMPLE_RATE = 44100; // desired rate - CD quality
const u32 PERIOD_SIZE = AUDIO_SAMPLE_RATE / FRAME_RATE;
const u32 SOUND_BUFFER_SIZE = PERIOD_SIZE * 8;
const u32 VIDEO_STATE_SIZE = 1024LL * 1024LL;
const u32 PIXEL_SIZE = 2;
const f32 PIXELS_PER_METER = 30.0f;

struct memory_arena_t {
    u32 size, used;
    u8* base;
};

void* _push_size(memory_arena_t* arena, u32 size) {
    assert(arena->used + size <= arena->size);
    u8* result = arena->base + arena->used;
    arena->used += size;
    return result;
};

void _zero_size(void* memory, size_t size) {
    for (int i = 0; i < size; ++i) {
        ((u8*)memory)[i] = 0;
    }
}

#define ARRAY_SIZE(array) ((sizeof(array)) / sizeof(array[0]))
#define PUSH_STRUCT(arena, type) (type *)_push_size(arena, sizeof(type))
#define PUSH_ARRAY(arena, count, type) (type *)_push_size(arena, count * sizeof(type))
#define ZERO_STRUCT(instance) _zero_size(&(instance), sizeof(instance))
;

struct platform_read_entire_file_result_t {
    u8* contents;
    u32 content_size;
};

struct camera_t {
    v2 center, to_top_left;
    f32 scaling;
};

struct spatial_partition_t {
    u32 capacity;
    u32* primary_entities;
    u32 primary_entity_count;
    u32* secondary_entities;
    u32 secondary_entity_count;
};

struct game_state_t {
    u32 initialized;

    sim_entity_t* entities;
    u32 entity_capacity;
    u32 entity_count;
    u64 latest_id;

    f32 spatial_partition_width;
    u32 spatial_partition_grid_width;
    spatial_partition_t* spatial_partitions;
    u32 spatial_partition_count;

    camera_t camera;

    memory_arena_t world_arena;
    phy_memory_t physics_arena;

    tex2 main_panel;
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

void game_update_and_render(game_state_t* game_state, f64 dt,
                            video_buffer_description_t buffer_description,
                            game_input_t game_input);

platform_read_entire_file_result_t platform_read_entire_file(const char * filename);

void platform_free_file_memory(void* memory);

#endif //PHYSICA_GAME_H

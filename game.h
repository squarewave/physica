//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_GAME_H
#define PHYSICA_GAME_H

#include "typedefs.h"
#include "constants.h"
#include "debug.h"
#include "renderer.h"
#include "tools.h"
#include "physica_math.h"
#include "sim_entity.h"
#include "hashmap.h"
#include "animation.h"
#include "background.h"
#include "animations.h"

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

enum world_direction_t {
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT,
};

struct rotation_state_t {
    b32 needs_reset;
    i32 target_direction;
    i32 current_direction;
    f32 progress;
};

struct game_state_t {
    u32 initialized;

    i64 next_entity_id;
    iterable_pool<sim_entity_t> entities;
    hashmap<entity_ties_t> collision_map;
    hashmap<sim_entity_t*> entity_map;

    f32 spatial_partition_width;
    u32 spatial_partition_grid_width;
    spatial_partition_t* spatial_partitions;
    u32 spatial_partition_count;

    camera_t main_camera;
    camera_t ui_camera;
    render_group_t main_render_group;
    render_group_t ui_render_group;

    memory_arena_t world_arena;
    memory_arena_t render_arena;

    phy_state_t physics_state;

    tex2 main_panel;

    input_memo_t input_memo;

    vec<animation_frame_t> animation_frames;
    animation_group_t main_animation_group;

    animations_list_t animations;

    tex2 terrain_1;
    tex2 wiz_bmp;

    animation_spec_t wiz_walking_right;
    animation_spec_t wiz_walking_left;
    animation_spec_t wiz_standing_right;
    animation_spec_t wiz_standing_left;
    animation_spec_t wiz_jumping_right;
    animation_spec_t wiz_jumping_left;

    animation_spec_t wiz_buzz;

    background_t background;

    u32 frame_buffer;
    u32 color_buffer;

    gl_programs_t gl_programs;

    sim_entity_t* player;
    v2 gravity_normal;
    f32 gravity_magnitude;

    rotation_state_t rotation_state;

    b32 paused;
    b32 advance_one_frame;
};

struct transient_state_t {
    v4 particle_center_data[MAX_PARTICLES];
    v2 particle_scaling_data[MAX_PARTICLES];
    rgba_t particle_color_data[MAX_PARTICLES];
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

struct mouse_input_t {
    v2 position; // position in pixel space
    v2 normalized_position;
    button_input_t left_click;
    button_input_t right_click;
    button_input_t middle_click;
    i32 wheel_delta;
};

struct keyboard_input_t {
    union {
        struct {
            button_input_t k0,k1,k2,k3,k4,k5,k6,k7,k8,k9;
            button_input_t a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z;

            button_input_t up,down,left,right;

        };
        button_input_t buttons[40];
    };
    b32 shift_down, alt_down, ctrl_down;
};

b32 was_pressed(button_input_t button) {
    return button.ended_down && button.transition_count;
}

b32 was_released(button_input_t button) {
    return (!button.ended_down) && button.transition_count;
}

struct game_input_t {
    union {
        struct {
            button_input_t button_a;
            button_input_t button_b;
            button_input_t button_x;
            button_input_t button_y;
            button_input_t button_l_bumper;
            button_input_t button_r_bumper;
            button_input_t button_l_stick;
            button_input_t button_r_stick;
        };
        button_input_t buttons[12];
    };
    analog_input_t analog_l_trigger, analog_r_trigger;
    joystick_input_t joystick_l, joystick_r;

    mouse_input_t mouse;
    keyboard_input_t keyboard;
};

struct platform_services_t; 

void game_update_and_render(platform_services_t platform,
                            game_state_t* game_state,
                            transient_state_t* transient_state,
                            f32 dt,
                            window_description_t window,
                            game_input_t* game_input,
                            tools_state_t* tools_state);

platform_read_entire_file_result_t platform_read_entire_file(const char * filename);

void platform_free_file_memory(void* memory);

#endif //PHYSICA_GAME_H

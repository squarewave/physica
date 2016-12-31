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

struct platform_read_entire_file_result_ {
    unsigned char* contents;
    i32 content_size;
};

struct spatial_partition_ {
    u32 capacity;
    u32* primary_entities;
    u32 primary_entity_count;
    u32* secondary_entities;
    u32 secondary_entity_count;
};

const u32 INPUT_FLAG_JUMP_STARTED = 1;

struct input_memo_ {
    u32 flags;

    f32 jump_elapsed;
};

enum world_direction_ {
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT,
};

struct rotation_state_ {
    b32 needs_reset;
    i32 target_direction;
    i32 current_direction;
    f32 progress;
};

struct game_state_ {
    u32 initialized;

    i64 next_entity_id;
    iterable_pool<sim_entity_> entities;
    hashmap<entity_ties_> collision_map;
    hashmap<sim_entity_*> entity_map;

    f32 spatial_partition_width;
    u32 spatial_partition_grid_width;
    spatial_partition_* spatial_partitions;
    u32 spatial_partition_count;

    camera_ main_camera;
    camera_ ui_camera;
    render_group_ main_render_group;
    render_group_ ui_render_group;

    memory_arena_ world_arena;
    memory_arena_ render_arena;

    phy_state_ physics_state;

    tex2 main_panel;

    input_memo_ input_memo;

    vec<animation_frame_> animation_frames;
    animation_group_ main_animation_group;

    animations_list_ animations;

    tex2 terrain_1;
    tex2 wiz_bmp;

    animation_spec_ wiz_walking_right;
    animation_spec_ wiz_walking_left;
    animation_spec_ wiz_standing_right;
    animation_spec_ wiz_standing_left;
    animation_spec_ wiz_jumping_right;
    animation_spec_ wiz_jumping_left;

    animation_spec_ wiz_buzz;

    background_ background;

    u32 frame_buffer;
    u32 color_buffer;

    gl_programs_ gl_programs;

    sim_entity_* player;
    v2 gravity_normal;
    f32 gravity_magnitude;

    rotation_state_ rotation_state;

    b32 paused;
    b32 advance_one_frame;
};

struct transient_state_ {
    v4 particle_center_data[MAX_PARTICLES];
    v2 particle_scaling_data[MAX_PARTICLES];
    rgba_ particle_color_data[MAX_PARTICLES];
};

struct button_input_ {
    u32 transition_count;
    b32 ended_down;
};

struct analog_input_ {
    f32 delta, value;
};

struct joystick_input_ {
    v2 delta, position;
};

struct mouse_input_ {
    v2 position; // position in pixel space
    v2 normalized_position;
    button_input_ left_click;
    button_input_ right_click;
    button_input_ middle_click;
    i32 wheel_delta;
};

struct keyboard_input_ {
    union {
        struct {
            button_input_ k0,k1,k2,k3,k4,k5,k6,k7,k8,k9;
            button_input_ a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z;

            button_input_ up,down,left,right;

        };
        button_input_ buttons[40];
    };
    b32 shift_down, alt_down, ctrl_down;
};

b32 was_pressed(button_input_ button) {
    return button.ended_down && button.transition_count;
}

b32 was_released(button_input_ button) {
    return (!button.ended_down) && button.transition_count;
}

struct game_input_ {
    union {
        struct {
            button_input_ button_a;
            button_input_ button_b;
            button_input_ button_x;
            button_input_ button_y;
            button_input_ button_l_bumper;
            button_input_ button_r_bumper;
            button_input_ button_l_stick;
            button_input_ button_r_stick;
        };
        button_input_ buttons[12];
    };
    analog_input_ analog_l_trigger, analog_r_trigger;
    joystick_input_ joystick_l, joystick_r;

    mouse_input_ mouse;
    keyboard_input_ keyboard;
};

struct platform_services_; 

void game_update_and_render(platform_services_ platform,
                            game_state_* game_state,
                            transient_state_* transient_state,
                            f32 dt,
                            window_description_ window,
                            game_input_* game_input,
                            tools_state_* tools_state);

platform_read_entire_file_result_ platform_read_entire_file(const char * filename);

void platform_free_file_memory(void* memory);

#endif //PHYSICA_GAME_H

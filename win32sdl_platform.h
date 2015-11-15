//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_SDL_PLATFORM_H
#define PHYSICA_SDL_PLATFORM_H

#include "gl/glew.h"
#include "gl/glu.h"

enum gamepad_buttons {
    BUTTON_A = 0,
    BUTTON_B = 1,
    BUTTON_X = 2,
    BUTTON_Y = 3,
    BUTTON_L_BUMPER = 4,
    BUTTON_R_BUMPER = 5,

    BUTTON_L_STICK = 9,
    BUTTON_R_STICK = 10,
};

enum gamepad_axes {
    L_STICK_X = 0,
    L_STICK_Y = 1,
    L_TRIGGER = 2,
    R_STICK_X = 3,
    R_STICK_Y = 4,
    R_TRIGGER = 5
};

struct offscreen_buffer_t
{
    void *memory;
    int width;
    int height;
    int pitch;
    int bytes_per_pixel;
};

struct basic_program_t {
    GLuint id, vao, ellipse_outline_vao, vbo, ibo;
    GLint transform_loc, color_loc, vertex_modelspace_loc;
    i32 ellipse_ibo_count;
};

struct texture_program_t {
    GLuint id, vao, vbo, uv_vbo, ibo;
    GLint transform_loc;
    GLint uv_transform_loc;
    GLint vertex_modelspace_loc;
    GLint vertex_uv_loc;
    GLint texture_sampler_loc;
};

struct platform_context_t {
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Texture* texture;
    SDL_GLContext gl_context;

    game_input_t* next_input;
    game_input_t* prev_input;

    SDL_GameController* controller_handle;
};

#define TASK_QUEUE_MAX_ENTRIES 256

struct task_queue_t;

typedef void task_callback_t(task_queue_t* queue, void* data); 

struct task_t {
    task_callback_t* callback;
    void* data;
};

struct task_queue_t {
    i32 volatile write_index;
    i32 volatile read_index;
    i32 volatile remaining;

    task_t tasks[TASK_QUEUE_MAX_ENTRIES];
    SDL_sem* semaphore;
};

typedef void start_task_func(task_queue_t* queue, task_callback_t* callback, void* data);
typedef void wait_on_queue_func(task_queue_t* queue);

struct platform_services_t {
    task_queue_t* primary_queue;
    task_queue_t* secondary_queue;
    task_queue_t* render_queue;
    start_task_func* start_task;
    wait_on_queue_func* wait_on_queue;
    basic_program_t basic_program;
    texture_program_t texture_program;
};

#endif //PHYSICA_SDL_PLATFORM_H

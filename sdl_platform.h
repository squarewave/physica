//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_SDL_PLATFORM_H
#define PHYSICA_SDL_PLATFORM_H

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

struct platform_context_t {
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Texture* texture;

    game_input_t* next_input;
    game_input_t* prev_input;

    SDL_Joystick* controller_handle;
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
};

#endif //PHYSICA_SDL_PLATFORM_H

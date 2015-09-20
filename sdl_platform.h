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

#endif //PHYSICA_SDL_PLATFORM_H

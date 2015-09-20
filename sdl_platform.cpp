//
// Created by doug on 4/16/15.
//

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "SDL2/SDL.h"

#include "game.h"
#include "sdl_platform.h"

const i32 width = 640;
const i32 height = 480;

void platform_free_file_memory(void* memory) {
    free(memory);
}

platform_read_entire_file_result_t platform_read_entire_file(const char * filename) {
    platform_read_entire_file_result_t result = {};

    FILE *f = fopen(filename, "rb");

    if (f == 0) {
        printf("Failed to open file %s\n", filename);
        return result;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = (char *)malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;

    result.contents = (u8 *)string;
    result.content_size = (u32) fsize;

    return result;
}

b32 handle_sdl_event(SDL_Event* event, platform_context_t* context) {
    b32 should_quit = false;
    switch (event->type) {
        case SDL_QUIT: {
            printf("SDL_QUIT\n");
            should_quit = true;
        } break;
        case SDL_WINDOWEVENT: {
            switch (event->window.event) {
                case SDL_WINDOWEVENT_RESIZED: {
                    printf("SDL_WINDOWEVENT_RESIZED (%d, %d)\n", event->window.data1, event->window.data2);
                } break;
                case SDL_WINDOWEVENT_EXPOSED: {
                    printf("SDL_WINDOWEVENT_EXPOSED\n");
                } break;
            } break;
        } break;
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            SDL_Keycode key_code = event->key.keysym.sym;
            b32 ended_down = event->key.state != SDL_RELEASED;

            if (event->type == SDL_KEYUP || event->key.repeat == 0) {
                switch (key_code) {
                    case SDLK_UP: {
                        context->next_input->button_a.ended_down = ended_down;
                        context->next_input->button_a.transition_count++;
                    } break;
                    case SDLK_DOWN: {
                        context->next_input->button_x.ended_down = ended_down;
                        context->next_input->button_x.transition_count++;
                    } break;
                    case SDLK_LEFT: {
                        context->next_input->joystick_l.position.x = ended_down ? -1.0f : 0.0f;
                    } break;
                    case SDLK_RIGHT: {
                        context->next_input->joystick_l.position.x = ended_down ? 1.0f : 0.0f;
                    } break;
                }
            }
        } break;
        case SDL_CONTROLLERDEVICEADDED: {
            if (context->controller_handle) {
                SDL_JoystickClose(context->controller_handle);
            }
            context->controller_handle = SDL_JoystickOpen(event->cdevice.which);
        } break;
        case SDL_CONTROLLERDEVICEREMOVED: {
            if (context->controller_handle) {
                SDL_JoystickClose(context->controller_handle);
            }
        } break;
    }

    SDL_Joystick* handle = context->controller_handle;
    if(handle && SDL_JoystickGetAttached(handle))
    {
        // NOTE: We have a controller with index ControllerIndex.
        b32 up = SDL_JoystickGetButton(handle, SDL_CONTROLLER_BUTTON_DPAD_UP);
        b32 down = SDL_JoystickGetButton(handle, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        b32 left = SDL_JoystickGetButton(handle, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        b32 right = SDL_JoystickGetButton(handle, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        b32 start = SDL_JoystickGetButton(handle, SDL_CONTROLLER_BUTTON_START);
        b32 back = SDL_JoystickGetButton(handle, SDL_CONTROLLER_BUTTON_BACK);
        b32 l_shoulder = SDL_JoystickGetButton(handle, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        b32 r_shoulder = SDL_JoystickGetButton(handle, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        b32 button_a = SDL_JoystickGetButton(handle, SDL_CONTROLLER_BUTTON_A);
        b32 button_b = SDL_JoystickGetButton(handle, SDL_CONTROLLER_BUTTON_B);
        b32 button_x = SDL_JoystickGetButton(handle, SDL_CONTROLLER_BUTTON_X);
        b32 button_y = SDL_JoystickGetButton(handle, SDL_CONTROLLER_BUTTON_Y);

        i16 stick_x = SDL_JoystickGetAxis(handle, SDL_CONTROLLER_AXIS_LEFTX);
        i16 stick_y = SDL_JoystickGetAxis(handle, SDL_CONTROLLER_AXIS_LEFTY);

        context->next_input->up.ended_down = up;
        context->next_input->down.ended_down = down;
        context->next_input->left.ended_down = left;
        context->next_input->right.ended_down = right;

        context->next_input->button_a.ended_down = button_a;
        context->next_input->button_b.ended_down = button_b;
        context->next_input->button_x.ended_down = button_x;
        context->next_input->button_y.ended_down = button_y;

        context->next_input->joystick_l.position.x = ((f32)stick_x) / 32768.0f;        
        context->next_input->joystick_l.position.y = ((f32)stick_y) / 32768.0f;        
    }

  return should_quit;
}

SDL_Joystick* find_controller_handle() {
    i32 num_joysticks = SDL_NumJoysticks();

    for (int i = 0; i < num_joysticks; ++i) {
        printf("%s\n", SDL_JoystickName(SDL_JoystickOpen(i)));
        return SDL_JoystickOpen(i);
    }

    printf("No controller found. Joysticks attached: %d\n", num_joysticks);
    return 0;
}

f32 get_seconds_elapsed(u64 old, u64 current) {
    return ((f32)(current - old) / (f32)(SDL_GetPerformanceFrequency()));
}

int main(int argc, char const *argv[]) {
    platform_context_t context = {};

    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        printf("Unable to init SDL: %s\n", SDL_GetError());
        return 1;
    }

    context.window = SDL_CreateWindow("hello",
                                      SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED,
                                      width,
                                      height,
                                      SDL_WINDOW_RESIZABLE);


    if (!context.window) {
        printf("Unable to create window: %s\n", SDL_GetError());
        return 1;
    }

    context.renderer = SDL_CreateRenderer(context.window, -1, 0);

    if (!context.renderer) {
        printf("Unable to create renderer: %s\n", SDL_GetError());
        return 1;
    }

    context.texture = SDL_CreateTexture(context.renderer,
                                        SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        width,
                                        height);

    if (!context.texture) {
        printf("Unable to create texture: %s\n", SDL_GetError());
        return 1;
    }

    void* pixels = malloc(width * height * 4);

    const u64 one_gig = 1024LL * 1024LL * 1024LL;
    void* game_memory = calloc(one_gig, sizeof(u8));

    context.controller_handle = find_controller_handle();

    u32 frame_index = 0;
    u32 s = 0;
    bool running = true;

    game_input_t prev_input = {};
    game_input_t next_input = {};
    context.next_input = &next_input;
    context.prev_input = &prev_input;

    u64 last_counter = SDL_GetPerformanceCounter();
    const f32 target_seconds_per_frame = 1.0f / (f32)FRAME_RATE;
    while(running) {
        f32 elapsed = get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter());
        if (elapsed < target_seconds_per_frame) {
            u32 sleep_time = ((target_seconds_per_frame - elapsed) * 1000) - 1;
            SDL_Delay(sleep_time);
            elapsed = get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter());
            assert(elapsed < target_seconds_per_frame);
            while (get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter()) <
                   target_seconds_per_frame) { }
        }
        f32 dt = get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter());
        last_counter = SDL_GetPerformanceCounter();

        ZERO_STRUCT(next_input);
        next_input.up.ended_down = prev_input.up.ended_down;
        next_input.down.ended_down = prev_input.down.ended_down;
        next_input.left.ended_down = prev_input.left.ended_down;
        next_input.right.ended_down = prev_input.right.ended_down;
        next_input.lshift.ended_down = prev_input.lshift.ended_down;
        next_input.rshift.ended_down = prev_input.rshift.ended_down;

        next_input.button_a.ended_down = prev_input.button_a.ended_down;
        next_input.button_b.ended_down = prev_input.button_b.ended_down;
        next_input.button_x.ended_down = prev_input.button_x.ended_down;
        next_input.button_y.ended_down = prev_input.button_y.ended_down;
        next_input.button_l_bumper.ended_down =
            prev_input.button_l_bumper.ended_down;
        next_input.button_r_bumper.ended_down =
            prev_input.button_r_bumper.ended_down;
        next_input.button_l_stick.ended_down =
            prev_input.button_l_stick.ended_down;
        next_input.button_r_stick.ended_down =
            prev_input.button_r_stick.ended_down;

        next_input.analog_l_trigger.value = prev_input.analog_l_trigger.value;
        next_input.analog_r_trigger.value = prev_input.analog_r_trigger.value;

        next_input.joystick_l.position = prev_input.joystick_l.position;
        next_input.joystick_r.position = prev_input.joystick_r.position;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (handle_sdl_event(&event, &context)) {
                running = false;
            }
        }

        next_input.analog_l_trigger.delta =
                next_input.analog_l_trigger.value -
                prev_input.analog_l_trigger.value;
        next_input.analog_r_trigger.delta =
                next_input.analog_r_trigger.value -
                prev_input.analog_r_trigger.value;

        next_input.joystick_l.delta =
                next_input.joystick_l.position - prev_input.joystick_l.position;
        next_input.joystick_r.delta =
                next_input.joystick_r.position - prev_input.joystick_r.position;

        video_buffer_description_t game_buffer = {};
        game_buffer.memory = pixels;
        game_buffer.width = width;
        game_buffer.height = height;
        game_buffer.pitch = width * 4;
        game_buffer.bytes_per_pixel = 4;

        game_update_and_render((game_state_t*)game_memory, dt, game_buffer, next_input);

        prev_input = next_input;

        if (SDL_UpdateTexture(context.texture,0, pixels, width * 4)) {
            printf("Unable to update SDL texture: %s\n", SDL_GetError());
            return 1;
        }

        SDL_RenderCopy(context.renderer, context.texture, 0, 0);

        SDL_RenderPresent(context.renderer);
    }

    return 0;
}
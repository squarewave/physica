//
// Created by doug on 4/16/15.
//

#include <math.h>
#include <time.h>
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>

#include "SDL2/SDL.h"

#include "game.h"
#include "sdl_platform.h"


void platform_start_task(task_queue_t* queue, task_callback_t* callback, void* data) {
    i32 next_write_index = (queue->write_index + 1) % TASK_QUEUE_MAX_ENTRIES;
    assert(next_write_index != queue->read_index);

    task_t* task = queue->tasks + queue->write_index;
    task->callback = callback;
    task->data = data;
    queue->write_index = next_write_index;
    __atomic_add_fetch(&queue->remaining, 1, __ATOMIC_SEQ_CST);
    SDL_SemPost(queue->semaphore);
}

b32 platform_execute_next_task(task_queue_t* queue) {
    b32 sleep = false;

    i32 original = queue->read_index;
    i32 read_index = (original + 1) % TASK_QUEUE_MAX_ENTRIES;
    if (original != queue->write_index) {
        if (__atomic_compare_exchange_n(&queue->read_index,
                                        &original, read_index,
                                        false,
                                        __ATOMIC_SEQ_CST,
                                        __ATOMIC_SEQ_CST)) {
            task_t* task = queue->tasks + original;
            task->callback(queue, task->data);
            __atomic_sub_fetch(&queue->remaining, 1, __ATOMIC_SEQ_CST);
        }
    } else {
        sleep = true;
    }

    return sleep;
}

void platform_wait_on_queue(task_queue_t* queue) {
    while (queue->remaining) {
        platform_execute_next_task(queue);
    }
}

int thread_func(void* ptr) {
    task_queue_t* queue = (task_queue_t*)ptr;

    while (true) {
        if (platform_execute_next_task(queue)) {
            SDL_SemWait(queue->semaphore);
        }
    }

    return 0;
}

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
    if (!fread(string, fsize, 1, f)) {
        printf("No results from fread\n");
    }
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
                    printf("SDL_WINDOWEVENT_RESIZED (%d, %d)\n", event->window.data1,
                           event->window.data2);
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

void segv_handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

void printer_task(task_queue_t* queue, void* data) {
    char* as_str = (char*)data;
    printf("%s\n", as_str);
}

const char* to_print = "hello, world";

int main(int argc, char const *argv[]) {
    signal(SIGSEGV, segv_handler);
    setlocale(LC_NUMERIC, "");

    platform_context_t context = {0};

    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        printf("Unable to init SDL: %s\n", SDL_GetError());
        return 1;
    }

    task_queue_t primary_queue = {0};
    primary_queue.semaphore = SDL_CreateSemaphore(0);
    task_queue_t secondary_queue = {0};
    secondary_queue.semaphore = SDL_CreateSemaphore(0);
    task_queue_t render_queue = {0};
    render_queue.semaphore = SDL_CreateSemaphore(0);

    platform_services_t platform = {0};
    platform.primary_queue = &primary_queue;
    platform.secondary_queue = &secondary_queue;
    platform.render_queue = &render_queue;
    platform.start_task = &platform_start_task;
    platform.wait_on_queue = &platform_wait_on_queue;

    for (int i = 0; i < 8; ++i) {
        char buffer[10];
        sprintf(buffer, "thread%d", i);
        SDL_CreateThread(thread_func, buffer, (void*)&render_queue);
    }

    context.window = SDL_CreateWindow("hello",
                                      SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED,
                                      START_WIDTH,
                                      START_HEIGHT,
                                      SDL_WINDOW_RESIZABLE);


    if (!context.window) {
        printf("Unable to create window: %s\n", SDL_GetError());
        return 1;
    }

    context.renderer = SDL_CreateRenderer(context.window, -1, SDL_RENDERER_PRESENTVSYNC);

    if (!context.renderer) {
        printf("Unable to create renderer: %s\n", SDL_GetError());
        return 1;
    }

    context.texture = SDL_CreateTexture(context.renderer,
                                        SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        START_WIDTH,
                                        START_HEIGHT);

    if (!context.texture) {
        printf("Unable to create texture: %s\n", SDL_GetError());
        return 1;
    }

    size_t pixel_bytes = START_WIDTH * START_HEIGHT * 4;
    void* pixels = malloc(pixel_bytes);
    // void* pixels = aligned_alloc(64, pixel_bytes + (pixel_bytes % 64));

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
        game_buffer.width = START_WIDTH;
        game_buffer.height = START_HEIGHT;
        game_buffer.pitch = START_WIDTH * 4;
        game_buffer.bytes_per_pixel = 4;

        game_update_and_render(platform, (game_state_t*)game_memory, dt, game_buffer, next_input);

        prev_input = next_input;

        if (SDL_UpdateTexture(context.texture,0, pixels, START_WIDTH * 4)) {
            printf("Unable to update SDL texture: %s\n", SDL_GetError());
            return 1;
        }

        SDL_RenderCopy(context.renderer, context.texture, 0, 0);

        SDL_RenderPresent(context.renderer);

        print_debug_log();
    }

    return 0;
}
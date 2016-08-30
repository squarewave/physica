//
// Created by doug on 4/16/15.
//

#include <unistd.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

// #include <unistd.h>
#include <locale.h>

#include "SDL2/SDL.h"

#include "game.h"
#include "linuxsdl_platform.h"

void platform_debug_print(char* str) {
    puts(str);
}

void platform_start_task(task_queue_t* queue, task_callback_t* callback, void* data) {
    i32 next_write_index = (queue->write_index + 1) % TASK_QUEUE_MAX_ENTRIES;
    assert_(next_write_index != queue->read_index);

    task_t* task = queue->tasks + queue->write_index;
    task->callback = callback;
    task->data = data;
    queue->write_index = next_write_index;
    __sync_fetch_and_add((i64 volatile*)&queue->remaining, 1);
    SDL_SemPost(queue->semaphore);
}

b32 platform_execute_next_task(task_queue_t* queue) {
    b32 sleep = false;

    i32 original = queue->read_index;
    i32 read_index = (original + 1) % TASK_QUEUE_MAX_ENTRIES;
    if (original != queue->write_index) {
        if (__sync_val_compare_and_swap((i64 volatile*)&queue->read_index,
                                        original,
                                        read_index) == original) {
            task_t* task = queue->tasks + original;
            task->callback(queue, task->data);
            __sync_fetch_and_add((i64 volatile*)&queue->remaining, -1);
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

    char *string = (char *)malloc((size_t)fsize + 1);
    if (!fread(string, (size_t)fsize, 1, f)) {
        printf("No results from fread\n");
    }
    fclose(f);

    string[fsize] = 0;

    result.contents = (u8 *)string;
    result.content_size = (i32) fsize;

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
                        context->next_input->keyboard.up.ended_down = ended_down;
                        context->next_input->keyboard.up.transition_count++;

                        context->next_input->button_a.ended_down = ended_down;
                        context->next_input->button_a.transition_count++;
                    } break;
                    case SDLK_DOWN: {
                        context->next_input->keyboard.down.ended_down = ended_down;
                        context->next_input->keyboard.down.transition_count++;

                        context->next_input->button_x.ended_down = ended_down;
                        context->next_input->button_x.transition_count++;
                    } break;
                    case SDLK_LEFT: {
                        context->next_input->keyboard.left.ended_down = ended_down;
                        context->next_input->keyboard.left.transition_count++;

                        context->next_input->joystick_l.position.x = ended_down ? 
                        -1.0f :
                        (context->next_input->keyboard.right.ended_down ? 1.0f : 0.0f);
                    } break;
                    case SDLK_RIGHT: {
                        context->next_input->keyboard.right.ended_down = ended_down;
                        context->next_input->keyboard.right.transition_count++;

                        context->next_input->joystick_l.position.x = ended_down ?
                        1.0f :
                        (context->next_input->keyboard.left.ended_down ? -1.0f : 0.0f);
                    } break;
                    case SDLK_0: {
                        context->next_input->keyboard.k0.ended_down = ended_down;
                        context->next_input->keyboard.k0.transition_count++;
                    } break;
                    case SDLK_1: {
                        context->next_input->keyboard.k1.ended_down = ended_down;
                        context->next_input->keyboard.k1.transition_count++;
                    } break;
                    case SDLK_2: {
                        context->next_input->keyboard.k2.ended_down = ended_down;
                        context->next_input->keyboard.k2.transition_count++;
                    } break;
                    case SDLK_3: {
                        context->next_input->keyboard.k3.ended_down = ended_down;
                        context->next_input->keyboard.k3.transition_count++;
                    } break;
                    case SDLK_4: {
                        context->next_input->keyboard.k4.ended_down = ended_down;
                        context->next_input->keyboard.k4.transition_count++;
                    } break;
                    case SDLK_5: {
                        context->next_input->keyboard.k5.ended_down = ended_down;
                        context->next_input->keyboard.k5.transition_count++;
                    } break;
                    case SDLK_6: {
                        context->next_input->keyboard.k6.ended_down = ended_down;
                        context->next_input->keyboard.k6.transition_count++;
                    } break;
                    case SDLK_7: {
                        context->next_input->keyboard.k7.ended_down = ended_down;
                        context->next_input->keyboard.k7.transition_count++;
                    } break;
                    case SDLK_8: {
                        context->next_input->keyboard.k8.ended_down = ended_down;
                        context->next_input->keyboard.k8.transition_count++;
                    } break;
                    case SDLK_9: {
                        context->next_input->keyboard.k9.ended_down = ended_down;
                        context->next_input->keyboard.k9.transition_count++;
                    } break;
                    case SDLK_a: {
                        context->next_input->keyboard.a.ended_down = ended_down;
                        context->next_input->keyboard.a.transition_count++;
                    } break;
                    case SDLK_b: {
                        context->next_input->keyboard.b.ended_down = ended_down;
                        context->next_input->keyboard.b.transition_count++;
                    } break;
                    case SDLK_c: {
                        context->next_input->keyboard.c.ended_down = ended_down;
                        context->next_input->keyboard.c.transition_count++;
                    } break;
                    case SDLK_d: {
                        context->next_input->keyboard.d.ended_down = ended_down;
                        context->next_input->keyboard.d.transition_count++;
                    } break;
                    case SDLK_e: {
                        context->next_input->keyboard.e.ended_down = ended_down;
                        context->next_input->keyboard.e.transition_count++;
                    } break;
                    case SDLK_f: {
                        context->next_input->keyboard.f.ended_down = ended_down;
                        context->next_input->keyboard.f.transition_count++;
                    } break;
                    case SDLK_g: {
                        context->next_input->keyboard.g.ended_down = ended_down;
                        context->next_input->keyboard.g.transition_count++;
                    } break;
                    case SDLK_h: {
                        context->next_input->keyboard.h.ended_down = ended_down;
                        context->next_input->keyboard.h.transition_count++;
                    } break;
                    case SDLK_i: {
                        context->next_input->keyboard.i.ended_down = ended_down;
                        context->next_input->keyboard.i.transition_count++;
                    } break;
                    case SDLK_j: {
                        context->next_input->keyboard.j.ended_down = ended_down;
                        context->next_input->keyboard.j.transition_count++;
                    } break;
                    case SDLK_k: {
                        context->next_input->keyboard.k.ended_down = ended_down;
                        context->next_input->keyboard.k.transition_count++;
                    } break;
                    case SDLK_l: {
                        context->next_input->keyboard.l.ended_down = ended_down;
                        context->next_input->keyboard.l.transition_count++;
                    } break;
                    case SDLK_m: {
                        context->next_input->keyboard.m.ended_down = ended_down;
                        context->next_input->keyboard.m.transition_count++;
                    } break;
                    case SDLK_n: {
                        context->next_input->keyboard.n.ended_down = ended_down;
                        context->next_input->keyboard.n.transition_count++;
                    } break;
                    case SDLK_o: {
                        context->next_input->keyboard.o.ended_down = ended_down;
                        context->next_input->keyboard.o.transition_count++;
                    } break;
                    case SDLK_p: {
                        context->next_input->keyboard.p.ended_down = ended_down;
                        context->next_input->keyboard.p.transition_count++;
                    } break;
                    case SDLK_q: {
                        context->next_input->keyboard.q.ended_down = ended_down;
                        context->next_input->keyboard.q.transition_count++;
                    } break;
                    case SDLK_r: {
                        context->next_input->keyboard.r.ended_down = ended_down;
                        context->next_input->keyboard.r.transition_count++;
                    } break;
                    case SDLK_s: {
                        context->next_input->keyboard.s.ended_down = ended_down;
                        context->next_input->keyboard.s.transition_count++;
                    } break;
                    case SDLK_t: {
                        context->next_input->keyboard.t.ended_down = ended_down;
                        context->next_input->keyboard.t.transition_count++;
                    } break;
                    case SDLK_u: {
                        context->next_input->keyboard.u.ended_down = ended_down;
                        context->next_input->keyboard.u.transition_count++;
                    } break;
                    case SDLK_v: {
                        context->next_input->keyboard.v.ended_down = ended_down;
                        context->next_input->keyboard.v.transition_count++;
                    } break;
                    case SDLK_w: {
                        context->next_input->keyboard.w.ended_down = ended_down;
                        context->next_input->keyboard.w.transition_count++;
                    } break;
                    case SDLK_x: {
                        context->next_input->keyboard.x.ended_down = ended_down;
                        context->next_input->keyboard.x.transition_count++;

                        context->next_input->analog_r_trigger.value = ended_down ? 1.0f : 0.0f;
                    } break;
                    case SDLK_y: {
                        context->next_input->keyboard.y.ended_down = ended_down;
                        context->next_input->keyboard.y.transition_count++;
                    } break;
                    case SDLK_z: {
                        context->next_input->keyboard.z.ended_down = ended_down;
                        context->next_input->keyboard.z.transition_count++;

                        context->next_input->analog_l_trigger.value = ended_down ? 1.0f : 0.0f;
                    } break;
                    case SDLK_LALT:
                    case SDLK_RALT: {
                        context->next_input->keyboard.alt_down = ended_down;
                    } break;
                    case SDLK_LCTRL:
                    case SDLK_RCTRL: {
                        context->next_input->keyboard.ctrl_down = ended_down;
                    } break;
                    case SDLK_LSHIFT:
                    case SDLK_RSHIFT: {
                        context->next_input->keyboard.shift_down = ended_down;
                    } break;
                    case SDLK_QUOTE: {
                    } break;
                    case SDLK_BACKQUOTE: {
                    } break;
                    case SDLK_BACKSLASH: {
                    } break;
                    case SDLK_BACKSPACE: {
                    } break;
                    case SDLK_COMMA: {
                    } break;
                    case SDLK_DELETE: {
                    } break;
                    case SDLK_LEFTBRACKET: {
                    } break;
                    case SDLK_MINUS: {
                    } break;
                    case SDLK_PERIOD: {
                    } break;
                    case SDLK_RETURN: {
                    } break;
                    case SDLK_RIGHTBRACKET: {
                    } break;
                    case SDLK_SEMICOLON: {
                    } break;
                    case SDLK_SLASH: {
                    } break;
                    case SDLK_SPACE: {
                    } break;
                    case SDLK_RETURN2: {
                    } break;
                    case SDLK_TAB: {
                    } break;

                    case SDLK_AC_BACK: {
                    } break;
                    case SDLK_AC_BOOKMARKS: {
                    } break;
                    case SDLK_AC_FORWARD: {
                    } break;
                    case SDLK_AC_HOME: {
                    } break;
                    case SDLK_AC_REFRESH: {
                    } break;
                    case SDLK_AC_SEARCH: {
                    } break;
                    case SDLK_AC_STOP: {
                    } break;
                    case SDLK_AGAIN: {
                    } break;
                    case SDLK_ALTERASE: {
                    } break;
                    case SDLK_APPLICATION: {
                    } break;
                    case SDLK_AUDIOMUTE: {
                    } break;
                    case SDLK_AUDIONEXT: {
                    } break;
                    case SDLK_AUDIOPLAY: {
                    } break;
                    case SDLK_AUDIOPREV: {
                    } break;
                    case SDLK_AUDIOSTOP: {
                    } break;
                    case SDLK_BRIGHTNESSDOWN: {
                    } break;
                    case SDLK_BRIGHTNESSUP: {
                    } break;
                    case SDLK_CALCULATOR: {
                    } break;
                    case SDLK_CANCEL: {
                    } break;
                    case SDLK_CAPSLOCK: {
                    } break;
                    case SDLK_CLEAR: {
                    } break;
                    case SDLK_CLEARAGAIN: {
                    } break;
                    case SDLK_COMPUTER: {
                    } break;
                    case SDLK_COPY: {
                    } break;
                    case SDLK_CRSEL: {
                    } break;
                    case SDLK_CURRENCYSUBUNIT: {
                    } break;
                    case SDLK_CURRENCYUNIT: {
                    } break;
                    case SDLK_CUT: {
                    } break;
                    case SDLK_DECIMALSEPARATOR: {
                    } break;
                    case SDLK_DISPLAYSWITCH: {
                    } break;
                    case SDLK_EJECT: {
                    } break;
                    case SDLK_END: {
                    } break;
                    case SDLK_EQUALS: {
                    } break;
                    case SDLK_ESCAPE: {
                    } break;
                    case SDLK_EXECUTE: {
                    } break;
                    case SDLK_EXSEL: {
                    } break;
                    case SDLK_F1: {
                    } break;
                    case SDLK_F10: {
                    } break;
                    case SDLK_F11: {
                    } break;
                    case SDLK_F12: {
                    } break;
                    case SDLK_F13: {
                    } break;
                    case SDLK_F14: {
                    } break;
                    case SDLK_F15: {
                    } break;
                    case SDLK_F16: {
                    } break;
                    case SDLK_F17: {
                    } break;
                    case SDLK_F18: {
                    } break;
                    case SDLK_F19: {
                    } break;
                    case SDLK_F2: {
                    } break;
                    case SDLK_F20: {
                    } break;
                    case SDLK_F21: {
                    } break;
                    case SDLK_F22: {
                    } break;
                    case SDLK_F23: {
                    } break;
                    case SDLK_F24: {
                    } break;
                    case SDLK_F3: {
                    } break;
                    case SDLK_F4: {
                    } break;
                    case SDLK_F5: {
                    } break;
                    case SDLK_F6: {
                    } break;
                    case SDLK_F7: {
                    } break;
                    case SDLK_F8: {
                    } break;
                    case SDLK_F9: {
                    } break;
                    case SDLK_FIND: {
                    } break;
                    case SDLK_HELP: {
                    } break;
                    case SDLK_HOME: {
                    } break;
                    case SDLK_INSERT: {
                    } break;
                    case SDLK_KBDILLUMDOWN: {
                    } break;
                    case SDLK_KBDILLUMTOGGLE: {
                    } break;
                    case SDLK_KBDILLUMUP: {
                    } break;
                    case SDLK_KP_0: {
                    } break;
                    case SDLK_KP_00: {
                    } break;
                    case SDLK_KP_000: {
                    } break;
                    case SDLK_KP_1: {
                    } break;
                    case SDLK_KP_2: {
                    } break;
                    case SDLK_KP_3: {
                    } break;
                    case SDLK_KP_4: {
                    } break;
                    case SDLK_KP_5: {
                    } break;
                    case SDLK_KP_6: {
                    } break;
                    case SDLK_KP_7: {
                    } break;
                    case SDLK_KP_8: {
                    } break;
                    case SDLK_KP_9: {
                    } break;
                    case SDLK_KP_A: {
                    } break;
                    case SDLK_KP_AMPERSAND: {
                    } break;
                    case SDLK_KP_AT: {
                    } break;
                    case SDLK_KP_B: {
                    } break;
                    case SDLK_KP_BACKSPACE: {
                    } break;
                    case SDLK_KP_BINARY: {
                    } break;
                    case SDLK_KP_C: {
                    } break;
                    case SDLK_KP_CLEAR: {
                    } break;
                    case SDLK_KP_CLEARENTRY: {
                    } break;
                    case SDLK_KP_COLON: {
                    } break;
                    case SDLK_KP_COMMA: {
                    } break;
                    case SDLK_KP_D: {
                    } break;
                    case SDLK_KP_DBLAMPERSAND: {
                    } break;
                    case SDLK_KP_DBLVERTICALBAR: {
                    } break;
                    case SDLK_KP_DECIMAL: {
                    } break;
                    case SDLK_KP_DIVIDE: {
                    } break;
                    case SDLK_KP_E: {
                    } break;
                    case SDLK_KP_ENTER: {
                    } break;
                    case SDLK_KP_EQUALS: {
                    } break;
                    case SDLK_KP_EQUALSAS400: {
                    } break;
                    case SDLK_KP_EXCLAM: {
                    } break;
                    case SDLK_KP_F: {
                    } break;
                    case SDLK_KP_GREATER: {
                    } break;
                    case SDLK_KP_HASH: {
                    } break;
                    case SDLK_KP_HEXADECIMAL: {
                    } break;
                    case SDLK_KP_LEFTBRACE: {
                    } break;
                    case SDLK_KP_LEFTPAREN: {
                    } break;
                    case SDLK_KP_LESS: {
                    } break;
                    case SDLK_KP_MEMADD: {
                    } break;
                    case SDLK_KP_MEMCLEAR: {
                    } break;
                    case SDLK_KP_MEMDIVIDE: {
                    } break;
                    case SDLK_KP_MEMMULTIPLY: {
                    } break;
                    case SDLK_KP_MEMRECALL: {
                    } break;
                    case SDLK_KP_MEMSTORE: {
                    } break;
                    case SDLK_KP_MEMSUBTRACT: {
                    } break;
                    case SDLK_KP_MINUS: {
                    } break;
                    case SDLK_KP_MULTIPLY: {
                    } break;
                    case SDLK_KP_OCTAL: {
                    } break;
                    case SDLK_KP_PERCENT: {
                    } break;
                    case SDLK_KP_PERIOD: {
                    } break;
                    case SDLK_KP_PLUS: {
                    } break;
                    case SDLK_KP_PLUSMINUS: {
                    } break;
                    case SDLK_KP_POWER: {
                    } break;
                    case SDLK_KP_RIGHTBRACE: {
                    } break;
                    case SDLK_KP_RIGHTPAREN: {
                    } break;
                    case SDLK_KP_SPACE: {
                    } break;
                    case SDLK_KP_TAB: {
                    } break;
                    case SDLK_KP_VERTICALBAR: {
                    } break;
                    case SDLK_KP_XOR: {
                    } break;
                    case SDLK_LGUI: {
                    } break;
                    case SDLK_MAIL: {
                    } break;
                    case SDLK_MEDIASELECT: {
                    } break;
                    case SDLK_MENU: {
                    } break;
                    case SDLK_MODE: {
                    } break;
                    case SDLK_MUTE: {
                    } break;
                    case SDLK_NUMLOCKCLEAR: {
                    } break;
                    case SDLK_OPER: {
                    } break;
                    case SDLK_OUT: {
                    } break;
                    case SDLK_PAGEDOWN: {
                    } break;
                    case SDLK_PAGEUP: {
                    } break;
                    case SDLK_PASTE: {
                    } break;
                    case SDLK_PAUSE: {
                    } break;
                    case SDLK_POWER: {
                    } break;
                    case SDLK_PRINTSCREEN: {
                    } break;
                    case SDLK_PRIOR: {
                    } break;
                    case SDLK_RGUI: {
                    } break;
                    case SDLK_SCROLLLOCK: {
                    } break;
                    case SDLK_SELECT: {
                    } break;
                    case SDLK_SEPARATOR: {
                    } break;
                    case SDLK_SLEEP: {
                    } break;
                    case SDLK_STOP: {
                    } break;
                    case SDLK_SYSREQ: {
                    } break;
                    case SDLK_THOUSANDSSEPARATOR: {
                    } break;
                    case SDLK_UNDO: {
                    } break;
                    case SDLK_UNKNOWN: {
                    } break;
                    case SDLK_VOLUMEDOWN: {
                    } break;
                    case SDLK_VOLUMEUP: {
                    } break;
                    case SDLK_WWW: {
                    } break;
                }
            }
        } break;
        case SDL_CONTROLLERDEVICEADDED: {
            if (context->controller_handle) {
                SDL_GameControllerClose(context->controller_handle);
            }
            context->controller_handle = SDL_GameControllerOpen(event->cdevice.which);
        } break;
        case SDL_CONTROLLERDEVICEREMOVED: {
            if (context->controller_handle) {
                SDL_GameControllerClose(context->controller_handle);
            }
        } break;
        case SDL_MOUSEMOTION: {
            i32 window_width = 0;
            i32 window_height = 0;
            SDL_GetWindowSize(context->window,
                              &window_width,
                              &window_height);

            context->next_input->mouse.position.x = (f32)event->motion.x;
            context->next_input->mouse.position.y = (f32)event->motion.y;
            context->next_input->mouse.normalized_position.x =
                ((f32)event->motion.x) / (0.5f * (f32)window_width) - 1.0f;
            context->next_input->mouse.normalized_position.y =
                -1.0f * (((f32)event->motion.y) / (0.5f * (f32)window_height) - 1.0f);
        } break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            b32 ended_down = event->button.state != SDL_RELEASED;

            switch (event->button.button) {
                case SDL_BUTTON_LEFT: {
                    context->next_input->mouse.left_click.ended_down = ended_down;
                    context->next_input->mouse.left_click.transition_count++;
                } break;
                case SDL_BUTTON_RIGHT: {
                    context->next_input->mouse.right_click.ended_down = ended_down;
                    context->next_input->mouse.right_click.transition_count++;
                } break;
                case SDL_BUTTON_MIDDLE: {
                    context->next_input->mouse.middle_click.ended_down = ended_down;
                    context->next_input->mouse.middle_click.transition_count++;
                } break;
            }
        } break;
        case SDL_MOUSEWHEEL: {
            context->next_input->mouse.wheel_delta = event->wheel.y;
        } break;
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP: {
            u8 button = event->cbutton.button;
            b32 ended_down = event->cbutton.state != SDL_RELEASED;

            switch (button) {
                case SDL_CONTROLLER_BUTTON_A: {
                    context->next_input->button_a.ended_down = ended_down;
                    context->next_input->button_a.transition_count++;
                } break;
                case SDL_CONTROLLER_BUTTON_B: {
                    context->next_input->button_b.ended_down = ended_down;
                    context->next_input->button_b.transition_count++;
                } break;
                case SDL_CONTROLLER_BUTTON_X: {
                    context->next_input->button_x.ended_down = ended_down;
                    context->next_input->button_x.transition_count++;
                } break;
                case SDL_CONTROLLER_BUTTON_Y: {
                    context->next_input->button_y.ended_down = ended_down;
                    context->next_input->button_y.transition_count++;
                } break;
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT: {
                    context->next_input->joystick_l.position.x = ended_down ? -1.0f : 0.0f;
                } break;
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: {
                    context->next_input->joystick_l.position.x = ended_down ? 1.0f : 0.0f;
                } break;
                case SDL_CONTROLLER_BUTTON_BACK: {
                } break;
                case SDL_CONTROLLER_BUTTON_GUIDE: {
                } break;
                case SDL_CONTROLLER_BUTTON_START: {
                } break;
                case SDL_CONTROLLER_BUTTON_LEFTSTICK: {
                } break;
                case SDL_CONTROLLER_BUTTON_RIGHTSTICK: {
                } break;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: {
                } break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: {
                } break;
                case SDL_CONTROLLER_BUTTON_DPAD_UP: {
                } break;
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN: {
                } break;
            }
        } break;
        case SDL_CONTROLLERAXISMOTION: {
            u8 axis = event->caxis.axis;
            i32 value = event->caxis.value;

            switch (axis) {
                case SDL_CONTROLLER_AXIS_LEFTX: {
                    context->next_input->joystick_l.position.x = ((f32)value) / 32768.0f;
                } break;
                case SDL_CONTROLLER_AXIS_LEFTY: {
                    context->next_input->joystick_l.position.y = ((f32)value) / 32768.0f;
                } break;
                case SDL_CONTROLLER_AXIS_RIGHTX: {
                    context->next_input->joystick_r.position.x = ((f32)value) / 32768.0f;
                } break;
                case SDL_CONTROLLER_AXIS_RIGHTY: {
                    context->next_input->joystick_r.position.y = ((f32)value) / 32768.0f;
                } break;
                case SDL_CONTROLLER_AXIS_TRIGGERLEFT: {
                    context->next_input->analog_l_trigger.value = ((f32)value) / 32768.0f;
                } break;
                case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: {
                    context->next_input->analog_r_trigger.value = ((f32)value) / 32768.0f;
                } break;
            }
        } break;
    }

    return should_quit;
}

SDL_GameController* find_controller_handle() {
    i32 num_joysticks = SDL_NumJoysticks();

    for (int i = 0; i < num_joysticks; ++i) {
        printf("%s\n", SDL_GameControllerName(SDL_GameControllerOpen(i)));
        return SDL_GameControllerOpen(i);
    }

    printf("No controller found. Joysticks attached: %d\n", num_joysticks);
    return 0;
}

f32 get_seconds_elapsed(u64 old, u64 current) {
    return ((f32)(current - old) / (f32)(SDL_GetPerformanceFrequency()));
}

void printer_task(task_queue_t* queue, void* data) {
    char* as_str = (char*)data;
    printf("%s\n", as_str);
}

 
void check_sdl_error(int line = -1) {
    const char *error = SDL_GetError();
    if (*error != '\0')
    {
        char buffer[1024];
        sprintf(buffer, "SDL Error: %s\n", error);
        if (line != -1)
            sprintf(buffer, " + line: %i\n", line);
        SDL_ClearError();
        platform_debug_print(buffer);
    }
}

int main() {
    setlocale(LC_NUMERIC, "");


    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        fprintf(stdout, "Current working dir: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }

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

    check_sdl_error(__LINE__);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    // SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    context.window = SDL_CreateWindow("Physica",
                                      SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED,
                                      START_WIDTH,
                                      START_HEIGHT,
                                      SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    check_sdl_error(__LINE__);

    context.gl_context = SDL_GL_CreateContext(context.window);
    check_sdl_error(__LINE__);

    SDL_GL_MakeCurrent(context.window, context.gl_context);
    check_sdl_error(__LINE__);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        platform_debug_print("Error initializing GLEW");
    }

    if( SDL_GL_SetSwapInterval(1) < 0 ){
        check_sdl_error(__LINE__);
    }
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(2.0f);

    glClearColor(0.784f, 0.8745f, 0.925f, 1.0f);

    // context.renderer = SDL_CreateRenderer(context.window, -1, SDL_RENDERER_PRESENTVSYNC);
    // check_sdl_error(__LINE__);

    // SDL_RendererInfo renderer_info;
    // SDL_GetRendererInfo(context.renderer, &renderer_info);

    // if ((renderer_info.flags & SDL_RENDERER_ACCELERATED) == 0 || 
    //     (renderer_info.flags & SDL_RENDERER_TARGETTEXTURE) == 0) {
    //     assert_(false);
    // }

    // if (!context.renderer) {
    //     printf("Unable to create renderer: %s\n", SDL_GetError());
    //     return 1;
    // }

    // context.texture = SDL_CreateTexture(context.renderer,
    //                                     SDL_PIXELFORMAT_ARGB8888,
    //                                     SDL_TEXTUREACCESS_STREAMING,
    //                                     START_WIDTH,
    //                                     START_HEIGHT);

    // if (!context.texture) {
    //     printf("Unable to create texture: %s\n", SDL_GetError());
    //     return 1;
    // }

    // size_t pixel_bytes = START_WIDTH * START_HEIGHT * 4;
    // void* pixels = malloc(pixel_bytes);
    // void* pixels = aligned_alloc(64, pixel_bytes + (pixel_bytes % 64));

    const u64 one_gig = 1024LL * 1024LL * 1024LL;
    void* game_memory = calloc(one_gig, sizeof(u8));
    void* transient_memory = calloc(one_gig / 2, sizeof(u8));
    void* tools_memory = calloc(one_gig / 2, sizeof(u8));

    context.controller_handle = find_controller_handle();

    bool running = true;

    game_input_t prev_input = {};
    game_input_t next_input = {};
    context.next_input = &next_input;
    context.prev_input = &prev_input;

    u64 last_counter = SDL_GetPerformanceCounter();
    const f32 target_seconds_per_frame = 1.0f / (f32)FRAME_RATE;
    while(running) {
        TIMED_BLOCK(allotted_time);

        f32 elapsed = get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter());
        if (elapsed < target_seconds_per_frame) {
            u32 sleep_time =
                (u32)(((target_seconds_per_frame - elapsed) * 1000) - 1.0f);
            SDL_Delay(sleep_time);
            elapsed = get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter());
            while (get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter()) <
                   target_seconds_per_frame) { }
        }

        TIMED_BLOCK(main_run_loop);

        // f32 dt = get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter());
        // {
        //     char char_buffer[256];
        //     sprintf(char_buffer, "\nfps: %d\n", (i32)(1.0f / dt));
        //     platform_debug_printA(char_buffer);
        // }

        last_counter = SDL_GetPerformanceCounter();

        ZERO_STRUCT(next_input);

        for (int i = 0; i < (i32)ARRAY_SIZE(next_input.buttons); ++i) {
            next_input.buttons[i].ended_down = prev_input.buttons[i].ended_down;
        }

        for (int i = 0; i < (i32)ARRAY_SIZE(next_input.keyboard.buttons); ++i) {
            next_input.keyboard.buttons[i].ended_down = prev_input.keyboard.buttons[i].ended_down;
        }

        next_input.analog_l_trigger.value = prev_input.analog_l_trigger.value;
        next_input.analog_r_trigger.value = prev_input.analog_r_trigger.value;

        next_input.joystick_l.position = prev_input.joystick_l.position;
        next_input.joystick_r.position = prev_input.joystick_r.position;

        next_input.mouse.position = prev_input.mouse.position;
        next_input.mouse.normalized_position = prev_input.mouse.normalized_position;

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

        window_description_t game_buffer = {};
        game_buffer.width = START_WIDTH;
        game_buffer.height = START_HEIGHT;

        game_update_and_render(platform,
                               (game_state_t*)game_memory,
                               (transient_state_t*)transient_memory,
                               target_seconds_per_frame,
                               game_buffer,
                               &next_input,
                               (tools_state_t*)tools_memory);

        prev_input = next_input;

        {
            TIMED_BLOCK(sdl_gl_swapwindow);
            SDL_GL_SwapWindow(context.window);
        }

        process_debug_log((tools_state_t*)tools_memory);
    }

    return 0;
}

//
// Created by doug on 4/16/15.
//

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>
#include <alsa/asoundlib.h>
#include <linux/joystick.h>

#define class class_name
#include <xcb/xcb_icccm.h>
#undef class

#include "game.h"
#include "linux_platform.h"

void platform_free_file_memory(void* memory) {
    free(memory);
}

platform_read_entire_file_result_t platform_read_entire_file(const char * filename) {
    platform_read_entire_file_result_t result = {};

    FILE *f = fopen(filename, "rb");

    if (f == 0)
    {
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

void render_weird_gradient(i32 offset_x, i32 offset_y, offscreen_buffer_t offscreen_buffer){
    u32* pixel = (u32*)offscreen_buffer.memory;
    for (int y = 0; y < offscreen_buffer.height; ++y)
    {
        for (int x = 0; x < offscreen_buffer.width; ++x)
        {
            *pixel = (u8)(x + offset_x) | ((u8)(y + offset_y) << 8);
            pixel++;
        }
    }
}

i16* init_alsa(snd_pcm_t** out_handle) {
    snd_pcm_t* pcm_handle;
    i32 err;
    snd_output_t* log;
    snd_pcm_hw_params_t* params;
    i16* sound_buffer = (i16*)malloc(SOUND_BUFFER_SIZE * 2);

    const char* default_hardware_device = (char*)"hw:0,0";

    snd_output_stdio_attach(&log, stderr, 0);
    err = snd_pcm_open(&pcm_handle, default_hardware_device, SND_PCM_STREAM_PLAYBACK, 0);
    if (!err) {
        snd_pcm_hw_params_alloca(&params);
        snd_pcm_hw_params_any(pcm_handle, params);

        snd_pcm_access_t access = SND_PCM_ACCESS_RW_INTERLEAVED;
        snd_pcm_hw_params_set_access(pcm_handle, params, access);

        snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE; // signed 16-bit little-endian
        snd_pcm_hw_params_set_format (pcm_handle, params, format);

        snd_pcm_hw_params_set_rate(pcm_handle, params, AUDIO_SAMPLE_RATE, 0);

        snd_pcm_hw_params_set_period_size(pcm_handle, params, PERIOD_SIZE, 0);
        snd_pcm_hw_params_set_buffer_size(pcm_handle, params, SOUND_BUFFER_SIZE);

        u32 channel_count = 2;
        snd_pcm_hw_params_set_channels(pcm_handle, params, channel_count);

        snd_pcm_hw_params(pcm_handle, params);
        snd_pcm_dump(pcm_handle, log);
    } else {
        // TODO(doug): logging
    }

    *out_handle = pcm_handle;
    return sound_buffer;
}

offscreen_buffer_t init_xcb_buffer(platform_context context) {
    offscreen_buffer_t offscreen_buffer = {};

    if (offscreen_buffer.xcb_image) {
        xcb_image_destroy(offscreen_buffer.xcb_image);
    }

    if (offscreen_buffer.xcb_pixmap_id) {
        xcb_free_pixmap(context.connection, offscreen_buffer.xcb_pixmap_id);
    }

    if (offscreen_buffer.xcb_gcontext_id) {
        xcb_free_gc(context.connection, offscreen_buffer.xcb_gcontext_id);
    }

    u8 pad = 32;
    u8 depth = 24;
    u8 bpp = 32;
    xcb_format_t *fmt = xcb_setup_pixmap_formats(context.setup);
    xcb_format_t *fmtend = fmt + xcb_setup_pixmap_formats_length(context.setup);
    while (fmt++ != fmtend)
    {
        if (fmt->scanline_pad == pad && fmt->depth == depth && fmt->bits_per_pixel == bpp)
        {
            break;
        }
    }
    if (fmt > fmtend) {
        fmt = 0;
    }

    size_t image_size = START_WIDTH * START_HEIGHT * (fmt->bits_per_pixel / 8);
    u8 *image_data = (u8 *)malloc(image_size);

    xcb_image_t* xcb_image =
        xcb_image_create(START_WIDTH,
                         START_HEIGHT,
                         XCB_IMAGE_FORMAT_Z_PIXMAP,
                         fmt->scanline_pad,
                         fmt->depth,
                         fmt->bits_per_pixel,
                         0,
                         (xcb_image_order_t)context.setup->image_byte_order,
                         XCB_IMAGE_ORDER_LSB_FIRST,
                         image_data,
                         image_size,
                         image_data);

    offscreen_buffer.width = START_WIDTH;
    offscreen_buffer.height = START_HEIGHT;
    offscreen_buffer.bytes_per_pixel = fmt->bits_per_pixel / 8;
    offscreen_buffer.pitch = offscreen_buffer.bytes_per_pixel *
        offscreen_buffer.width;

    offscreen_buffer.xcb_image = xcb_image;
    offscreen_buffer.memory = offscreen_buffer.xcb_image->data;

    offscreen_buffer.xcb_pixmap_id = xcb_generate_id(context.connection);
    xcb_create_pixmap(context.connection, fmt->depth,
                      offscreen_buffer.xcb_pixmap_id, context.window,
                      offscreen_buffer.width, offscreen_buffer.height);
    xcb_flush(context.connection);

    offscreen_buffer.xcb_gcontext_id = xcb_generate_id(context.connection);
    xcb_create_gc(context.connection, offscreen_buffer.xcb_gcontext_id,
                  offscreen_buffer.xcb_pixmap_id, 0, 0);

    xcb_flush(context.connection);

    xcb_image_put(context.connection,
                  offscreen_buffer.xcb_pixmap_id,
                  offscreen_buffer.xcb_gcontext_id,
                  offscreen_buffer.xcb_image,
                  0, 0, 0);
    xcb_flush(context.connection);

    return offscreen_buffer;
}

int main(int argc, char const *argv[]) {
    platform_context context = {};

    context.connection = xcb_connect(0, 0);

    context.key_symbols = xcb_key_symbols_alloc(context.connection);

    context.setup = xcb_get_setup(context.connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(context.setup);
    xcb_screen_t* screen = iter.data;
    context.window = xcb_generate_id(context.connection);

    u32 mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    u32 valwin[2] = {
            0xffffffff,
            XCB_EVENT_MASK_EXPOSURE |
            XCB_EVENT_MASK_KEY_PRESS |
            XCB_EVENT_MASK_KEY_RELEASE
    };

    xcb_create_window(context.connection,
                      XCB_COPY_FROM_PARENT,
                      context.window,
                      screen->root,
                      0, 0,
                      START_WIDTH, START_HEIGHT,
                      10,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual,
                      mask,
                      valwin);

    u16 dw_name_length = 16;
    u16 proto_name_length = 12;

    xcb_intern_atom_cookie_t wm_delete_window_cookie =
            xcb_intern_atom(context.connection,
                            0,
                            dw_name_length,
                            "WM_DELETE_WINDOW");
    xcb_intern_atom_cookie_t wm_protocols_cookie =
            xcb_intern_atom(context.connection,
                            0,
                            proto_name_length,
                            "WM_PROTOCOLS");

    xcb_intern_atom_reply_t* wm_delete_window_cookie_reply =
            xcb_intern_atom_reply(context.connection,
                                  wm_delete_window_cookie,
                                  0);
    xcb_intern_atom_reply_t* wm_protocols_cookie_reply =
            xcb_intern_atom_reply(context.connection, wm_protocols_cookie, 0);

    xcb_atom_t wm_protocols_atom = {};
    xcb_atom_t wm_delete_window_atom = {};

    if (wm_protocols_cookie_reply)
    {
        wm_protocols_atom = wm_protocols_cookie_reply->atom;
    } else {
        // TODO(doug): Diagnostics
    }

    if (wm_delete_window_cookie_reply)
    {
        wm_delete_window_atom = wm_delete_window_cookie_reply->atom;
    } else {
        // TODO(doug): Diagnostics
    }

    xcb_change_property(context.connection,
                        XCB_PROP_MODE_REPLACE,
                        context.window,
                        wm_protocols_atom,
                        4, 32, 1,
                        &wm_delete_window_atom);

    char title[] = "wiz biz";
    xcb_change_property (context.connection,
                         XCB_PROP_MODE_REPLACE,
                         context.window,
                         XCB_ATOM_WM_NAME,
                         XCB_ATOM_STRING,
                         8,
                         7,
                         title );

    xcb_map_window(context.connection, context.window);
    xcb_flush(context.connection);

    const u64 one_gig = 1024LL * 1024LL * 1024LL;
    void* game_memory = calloc(one_gig, sizeof(u8));

    offscreen_buffer_t offscreen_buffer = init_xcb_buffer(context);

    snd_pcm_t* pcm_handle;
    i16* sound_buffer = init_alsa(&pcm_handle);

    u32 frame_index = 0;
    u32 s = 0;
    bool running = true;
    timespec previous_time = {};
    timespec next_time = {};
    clock_gettime(CLOCK_MONOTONIC, &previous_time);

    game_input_t prev_input = {};
    game_input_t next_input = {};

    i32 game_pad = open("/dev/input/js0", O_NONBLOCK);

    const u32 target_nanoseconds = 1000000000LL / FRAME_RATE;
    while(running) {
        clock_gettime(CLOCK_MONOTONIC, &next_time);
        timespec target_time;
        target_time.tv_nsec = previous_time.tv_nsec + target_nanoseconds;
        target_time.tv_sec = previous_time.tv_sec;
        const i64 one_billion = 1000 * 1000 * 1000;
        if (target_time.tv_nsec > one_billion) {
            target_time.tv_nsec -= one_billion;
            target_time.tv_sec += 1;
        }

        i64 delta_tn =
                (next_time.tv_nsec - previous_time.tv_nsec) +
                ((next_time.tv_sec - previous_time.tv_sec) * one_billion);

        i64 target_delta_tn =
                (target_time.tv_nsec - previous_time.tv_nsec) +
                ((target_time.tv_sec - previous_time.tv_sec) * one_billion);

        while (delta_tn < target_delta_tn) {
            clock_gettime(CLOCK_MONOTONIC, &next_time);
            delta_tn =
                    (next_time.tv_nsec - previous_time.tv_nsec) +
                    ((next_time.tv_sec - previous_time.tv_sec) * one_billion);
        }
        previous_time = next_time;

        xcb_generic_event_t *event = 0;
        xcb_generic_event_t *peek_event = 0;

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

        js_event game_pad_event;

// can't do a do...while :(
#define __CASE_UPDATE_GP_BUTTON(button, input_name) \
  case (button): { \
    next_input.input_name.ended_down = game_pad_event.value; \
    next_input.input_name.transition_count++; \
  } break;

        while (read(game_pad, &game_pad_event, sizeof(game_pad_event)) > 0) {
            if (game_pad_event.type == JS_EVENT_BUTTON) {
                switch (game_pad_event.number) {
                    __CASE_UPDATE_GP_BUTTON(BUTTON_A, button_a)
                    __CASE_UPDATE_GP_BUTTON(BUTTON_B, button_b)
                    __CASE_UPDATE_GP_BUTTON(BUTTON_X, button_x)
                    __CASE_UPDATE_GP_BUTTON(BUTTON_Y, button_y)
                    __CASE_UPDATE_GP_BUTTON(BUTTON_R_BUMPER, button_r_bumper)
                    __CASE_UPDATE_GP_BUTTON(BUTTON_L_BUMPER, button_l_bumper)
                    __CASE_UPDATE_GP_BUTTON(BUTTON_L_STICK, button_l_stick)
                    __CASE_UPDATE_GP_BUTTON(BUTTON_R_STICK, button_r_stick)
                }
            } else if (game_pad_event.type == JS_EVENT_AXIS) {
                switch (game_pad_event.number) {
                    case L_STICK_X: {
                        // don't use deadzones - we want game code to manage this
                        next_input.joystick_l.position.x =
                            ((f32)game_pad_event.value) / 32768.0f;
                    } break;
                    case L_STICK_Y: {
                        next_input.joystick_l.position.y =
                            ((f32)game_pad_event.value) / -32768.0f;
                    } break;
                    case R_STICK_X: {
                        next_input.joystick_r.position.x =
                            ((f32)game_pad_event.value) / 32768.0f;
                    } break;
                    case R_STICK_Y: {
                        next_input.joystick_r.position.y =
                            ((f32)game_pad_event.value) / -32768.0f;
                    } break;
                    case L_TRIGGER: {
                        next_input.analog_l_trigger.value =
                                (((f32)game_pad_event.value) / 65536.0f) + 0.5f;
                    } break;
                    case R_TRIGGER: {
                        next_input.analog_r_trigger.value =
                                (((f32)game_pad_event.value) / 65536.0f) + 0.5f;
                    } break;
                }
            }
        }

        while ((event = peek_event) ||
                (event = xcb_poll_for_event(context.connection))) {
            peek_event = 0;

            // NOTE(nbm): The high-order bit of response_type is whether the event
            // is synthetic.  I'm not sure I care, but let's grab it in case.
            b32 synthetic_event = (event->response_type & 0x80) != 0;

            u8 response_type = event->response_type & ~0x80;
            switch(response_type) {
                case XCB_KEY_PRESS: {
                    xcb_keycode_t key_code = ((xcb_key_press_event_t*)event)->detail;
                    xcb_keysym_t keysym = xcb_key_symbols_get_keysym(context.key_symbols, key_code, 0);
                    if (keysym == XK_Up) {
                        if (!prev_input.button_a.ended_down) {
                            next_input.button_a.ended_down = true;
                            next_input.button_a.transition_count++;
                        }
                    } else if (keysym == XK_Down) {
                        if (!prev_input.button_x.ended_down) {
                            next_input.button_x.ended_down = true;
                            next_input.button_x.transition_count++;
                        }
                    } else if (keysym == XK_Left) {
                        next_input.joystick_l.position.x = -1.0f;
                    } else if (keysym == XK_Right) {
                        next_input.joystick_l.position.x = 1.0f;
                    } else if (keysym == XK_z) {
                        if (!prev_input.lshift.ended_down) {
                            next_input.lshift.ended_down = true;
                            next_input.lshift.transition_count++;
                        }
                    } else if (keysym == XK_x) {
                        if (!prev_input.rshift.ended_down) {
                            next_input.rshift.ended_down = true;
                            next_input.rshift.transition_count++;
                        }
                    }
                } break;
                case XCB_KEY_RELEASE: {
                    xcb_keycode_t key_code = ((xcb_key_release_event_t*)event)->detail;

                    event = 0;

                    if ((peek_event = xcb_poll_for_event(context.connection))) {
                        u8 peek_response_type = peek_event->response_type & ~0x80;
                        if (peek_response_type == XCB_KEY_PRESS) {
                            xcb_keycode_t peek_key_code = ((xcb_key_press_event_t*)peek_event)->detail;
                            if (peek_key_code == key_code) {
                                peek_event = 0;
                                break;
                            }
                        }
                    }

                    xcb_keysym_t keysym = xcb_key_symbols_get_keysym(context.key_symbols, key_code, 0);
                    if (keysym == XK_Up) {
                        if (prev_input.button_a.ended_down) {
                            next_input.button_a.ended_down = false;
                            next_input.button_a.transition_count++;
                        }
                    } else if (keysym == XK_Down) {
                        if (prev_input.button_x.ended_down) {
                            next_input.button_x.ended_down = false;
                            next_input.button_x.transition_count++;
                        }
                    } else if (keysym == XK_Left) {
                        next_input.joystick_l.position.x = 0.0f;
                    } else if (keysym == XK_Right) {
                        next_input.joystick_l.position.x = 0.0f;
                    } else if (keysym == XK_z) {
                        if (prev_input.lshift.ended_down) {
                            next_input.lshift.ended_down = false;
                            next_input.lshift.transition_count++;
                        }
                    } else if (keysym == XK_x) {
                        if (prev_input.rshift.ended_down) {
                            next_input.rshift.ended_down = false;
                            next_input.rshift.transition_count++;
                        }
                    }
                } break;
                case XCB_CLIENT_MESSAGE: {
                    xcb_client_message_event_t* client_message_event =
                        (xcb_client_message_event_t*)event;

                    if (client_message_event->type == wm_protocols_atom)
                    {
                        if (client_message_event->data.data32[0] ==
                                wm_delete_window_atom)
                        {
                            running = false;
                        }
                    }
                } break;
                default: {
                } break;
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
        game_buffer.memory = offscreen_buffer.xcb_image->data;
        game_buffer.width = offscreen_buffer.width;
        game_buffer.height = offscreen_buffer.height;
        game_buffer.pitch = offscreen_buffer.pitch;
        game_buffer.bytes_per_pixel = offscreen_buffer.bytes_per_pixel;

        // f32 dt = (f32)(((f64)delta_tn) / (f64)(1000LL * 1000LL * 1000LL));
        f32 dt = (f32)(((f64)target_delta_tn) / (f64)(1000LL * 1000LL * 1000LL));

        game_update_and_render((game_state_t*)game_memory, dt, game_buffer, next_input);
        prev_input = next_input;

        xcb_image_put(context.connection, offscreen_buffer.xcb_pixmap_id,
                      offscreen_buffer.xcb_gcontext_id, offscreen_buffer.xcb_image, 0, 0, 0);
        xcb_flush(context.connection);

        xcb_copy_area(context.connection, offscreen_buffer.xcb_pixmap_id, context.window,
                      offscreen_buffer.xcb_gcontext_id, 0,0, 0, 0, offscreen_buffer.xcb_image->width,
                      offscreen_buffer.xcb_image->height);
        xcb_flush(context.connection);

        // i32 available_frames = snd_pcm_avail(pcm_handle);

        // if (available_frames > 0) {
        //   if (available_frames < SOUND_BUFFER_SIZE) {
        //     game_get_sound_samples((game_state_t*)game_memory, sound_buffer, available_frames);
        //   } else {
        //     game_get_sound_samples((game_state_t*)game_memory, sound_buffer, SOUND_BUFFER_SIZE);
        //   }

        //   i32 frames_written = snd_pcm_writei(pcm_handle, sound_buffer, available_frames);
        //   if (frames_written < 0) {
        //     frames_written = snd_pcm_recover(pcm_handle, frames_written, 0);
        //     if (frames_written < 0) {
        //       //TODO(doug): logging
        //     }
        //   }
        // }
    }

    xcb_flush(context.connection);
    xcb_disconnect(context.connection);
    return 0;
}
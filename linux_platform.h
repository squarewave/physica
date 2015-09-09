//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_LINUX_PLATFORM_H
#define PHYSICA_LINUX_PLATFORM_H

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
    xcb_image_t *xcb_image;
    xcb_pixmap_t xcb_pixmap_id;
    xcb_gcontext_t xcb_gcontext_id;

    void *memory;
    int width;
    int height;
    int pitch;
    int bytes_per_pixel;
};

struct platform_context {
    const xcb_setup_t *setup;
    xcb_format_t *fmt;
    xcb_connection_t *connection;
    xcb_window_t window;
    xcb_key_symbols_t* key_symbols;
    xcb_atom_t wm_protocols;
    xcb_atom_t wm_delete_window;
};

#endif //PHYSICA_LINUX_PLATFORM_H

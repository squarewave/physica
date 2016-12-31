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

struct offscreen_buffer_
{
    xcb_image_ *xcb_image;
    xcb_pixmap_ xcb_pixmap_id;
    xcb_gcontext_ xcb_gcontext_id;

    void *memory;
    int width;
    int height;
    int pitch;
    int bytes_per_pixel;
};

struct platform_context {
    const xcb_setup_ *setup;
    xcb_format_ *fmt;
    xcb_connection_ *connection;
    xcb_window_ window;
    xcb_key_symbols_* key_symbols;
    xcb_atom_ wm_protocols;
    xcb_atom_ wm_delete_window;
};

#endif //PHYSICA_LINUX_PLATFORM_H

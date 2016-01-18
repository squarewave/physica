#ifndef GAME_TOOLS_H_
#define GAME_TOOLS_H_

#include "debug.h"

struct game_state_t;
struct game_input_t;

enum ui_element_type_t {
    UI_NONE,
    UI_MENU,
    UI_DRAGGABLE_WIDGET,
    UI_TOGGLE_MENU,
    UI_TOGGLE_WIREFRAMES,
    UI_TOGGLE_AABB_TREE,
    UI_TOGGLE_PERFORMANCE,
    UI_COLOR_PICKER,
};

struct ui_element_t;

struct draggable_widget_options_t {
    v2 initial_mouse_position;
    ui_element_t* child;
};

struct menu_options_t {
    b32 active;
    ui_element_t* children;
    i32 child_count;
};

struct color_picker_options_t {
    v3 hsv;
};

struct ui_element_t {
    u32 type;
    ui_element_t* parent;
    v2 offset;
    v2 size;
    union {
        menu_options_t menu;
        color_picker_options_t color_picker;
        draggable_widget_options_t draggable_widget;
    };
};

struct tools_state_t {
    v2 proper_camera_center;
    v2 zoom_camera_offset;
    v2 debug_world_coord;
    i32 zoom_level;
    debug_state_t debug_state;
    vec<ui_element_t> ui_elements;

    ui_element_t* root_element;
    ui_element_t* active_element;
    ui_element_t* hover_element;

    i32 selected_render_item;
};

void tools_init(tools_state_t* tools_state);

void tools_update_and_render(game_state_t* game_state,
                             tools_state_t* tools_state,
                             f32 dt,
                             window_description_t window,
                             game_input_t* game_input);

#endif

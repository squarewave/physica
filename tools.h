#ifndef GAME_TOOLS_H_
#define GAME_TOOLS_H_

#include "debug.h"

struct game_state_;
struct game_input_;

enum ui_element_type_ {
    UI_NONE,
    UI_MENU,
    UI_DRAGGABLE_WIDGET,
    UI_TOGGLE_MENU,
    UI_TOGGLE_WIREFRAMES,
    UI_TOGGLE_AABB_TREE,
    UI_TOGGLE_PERFORMANCE,
    UI_COLOR_PICKER,
};

struct ui_element_;

struct draggable_widget_options_ {
    v2 drag_anchor;
    ui_element_* child;
};

struct menu_options_ {
    b32 active;
    ui_element_* children;
    i32 child_count;
};

struct color_picker_options_ {
    v3 hsv;
};

struct ui_element_ {
    u32 type;
    ui_element_* parent;
    v2 offset;
    v2 size;
    union {
        menu_options_ menu;
        color_picker_options_ color_picker;
        draggable_widget_options_ draggable_widget;
    };
};

struct tools_state_ {
    v2 proper_camera_center;
    v2 zoom_camera_offset;
    v2 debug_world_coord;
    i32 zoom_level;
    debug_state_ debug_state;
    vec<ui_element_> ui_elements;

    ui_element_* root_element;
    ui_element_* active_element;
    ui_element_* hover_element;

    i32 selected_render_item;
};

void tools_init(tools_state_* tools_state);

void tools_update_and_render(game_state_* game_state,
                             tools_state_* tools_state,
                             f32 dt,
                             window_description_ window,
                             game_input_* game_input);

#endif

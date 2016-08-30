#include "camera.h"


void tools_init(tools_state_t* tools_state) {
    debug_init(tools_state);

    const i32 max_ui_elements = 1024;
    tools_state->ui_elements.values = (ui_element_t*)malloc(max_ui_elements *
                                                            sizeof(ui_element_t));
    tools_state->ui_elements.capacity = max_ui_elements;

    ui_element_t* main_menu = tools_state->root_element = tools_state->ui_elements.push({0});
    main_menu->type = UI_MENU;    
    main_menu->parent = 0;
    main_menu->offset = v2 {0.0f, 0.0f};
    main_menu->size = v2 {0.0f, 0.0f};
    main_menu->menu.active = false;
    i32 main_menu_item_count = 5;
    main_menu->menu.children = tools_state->ui_elements.push_many(main_menu_item_count);
    main_menu->menu.child_count = main_menu_item_count;

    ui_element_t* main_menu_toggle = &main_menu->menu.children[0];
    main_menu_toggle->type = UI_TOGGLE_MENU;
    main_menu_toggle->parent = main_menu;
    main_menu_toggle->offset = v2 {8.0f, 8.0f};
    main_menu_toggle->size = v2 {40.0f, 40.0f};

    ui_element_t* wireframes_toggle = &main_menu->menu.children[1];
    wireframes_toggle->type = UI_TOGGLE_WIREFRAMES;
    wireframes_toggle->parent = main_menu;
    wireframes_toggle->offset = v2 {8.0f, 56.0f};
    wireframes_toggle->size = v2 {40.0f, 40.0f};

    ui_element_t* aabb_tree_toggle = &main_menu->menu.children[2];
    aabb_tree_toggle->type = UI_TOGGLE_AABB_TREE;
    aabb_tree_toggle->parent = main_menu;
    aabb_tree_toggle->offset = v2 {8.0f, 104.0f};
    aabb_tree_toggle->size = v2 {40.0f, 40.0f};

    ui_element_t* performance_toggle = &main_menu->menu.children[3];
    performance_toggle->type = UI_TOGGLE_PERFORMANCE;
    performance_toggle->parent = main_menu;
    performance_toggle->offset = v2 {8.0f, 152.0f};
    performance_toggle->size = v2 {40.0f, 40.0f};

    ui_element_t* draggable = &main_menu->menu.children[4];
    draggable->type = UI_DRAGGABLE_WIDGET;
    draggable->parent = main_menu;
    draggable->offset = v2 {8.0f, 200.0f};
    draggable->size = v2 {208.0f, 208.0f};

    ui_element_t* color_picker = draggable->draggable_widget.child =
        tools_state->ui_elements.push({0});
    color_picker->type = UI_COLOR_PICKER;
    color_picker->parent = draggable;
    color_picker->offset = v2 {4.0f, 4.0f};
    color_picker->size = v2 {200.0f, 180.0f};

    tools_state->selected_render_item = -1;
}

inline f32
get_factor(i32 zoom_level) {
    switch(zoom_level) {
        case -6: { return 0.03125f; }
        case -5: { return 0.0625f; }
        case -4: { return 0.125f; }
        case -3: { return 0.25f; }
        case -2: { return 0.5f; }
        case -1: { return 0.75f; }
        case  0: { return 1.0f; }
        case  1: { return 1.25f; }
        case  2: { return 1.5f; }
        case  3: { return 2.0f; }
        case  4: { return 3.0f; }
        case  5: { return 4.0f; }
        case  6: { return 6.0f; }
    }
    assert_(false);
    return 0;
}

struct ui_draw_stack_item_t {
    ui_element_t* element;
    v2 parent_offset;
};

void draw_tools_menu(game_state_t* game_state,
                     tools_state_t* tools_state,
                     window_description_t window,
                     game_input_t* game_input) {
    TIMED_FUNC();

    i32 stack_index = 0;
    ui_draw_stack_item_t stack[MEDIUM_STACK_SIZE] = {0};

    m3x3 inv_ui_view = get_inverse_view_transform_3x3(game_state->ui_camera);
    v2 mouse_position = inv_ui_view * game_input->mouse.normalized_position;

    ui_element_t* hover_element = 0;

    ui_draw_stack_item_t root_item;
    root_item.element = tools_state->root_element;
    root_item.parent_offset = v2{0};
    stack[stack_index++] = root_item;

    while (stack_index) {
        ui_draw_stack_item_t item = stack[--stack_index];

        v2 offset = item.element->offset + item.parent_offset;
        rect draw_rect = rect { offset, offset + item.element->size };

        b32 hovering = is_in_rect(mouse_position, draw_rect);

        switch (item.element->type) {
            case UI_MENU: {
                for (int i = 0; i < item.element->menu.child_count; ++i) {
                    ui_element_t* sub_element = &item.element->menu.children[i];
                    if (item.element->menu.active || sub_element->type == UI_TOGGLE_MENU) {
                        ui_draw_stack_item_t new_item;
                        new_item.element = sub_element;
                        new_item.parent_offset = item.parent_offset + item.element->offset;
                        stack[stack_index++] = new_item;
                    }
                }
            } break;
            case UI_DRAGGABLE_WIDGET: {
                v2 mouse_delta = (item.element == tools_state->active_element) ?
                    (mouse_position - item.element->draggable_widget.initial_mouse_position) :
                    v2 {0};
                draw_rect.min += mouse_delta;
                draw_rect.max += mouse_delta;
                ui_draw_stack_item_t new_item;
                new_item.element = item.element->draggable_widget.child;
                new_item.parent_offset = item.parent_offset + mouse_delta +
                    item.element->offset;
                stack[stack_index++] = new_item;
                push_rect(&game_state->ui_render_group,
                                  color_t{0.5f,0.5f,0.5f},
                                  draw_rect);
            } break;
            case UI_TOGGLE_MENU: {
                push_rect(&game_state->ui_render_group,
                          hovering ? color_t {0.2f, 0.2f, 0.2f} : color_t {0.0f, 0.0f, 0.0f},
                          draw_rect);
            } break;
            case UI_TOGGLE_WIREFRAMES: {
                push_rect(&game_state->ui_render_group,
                          hovering ? color_t {0.2f, 0.6f, 0.2f} : color_t {0.0f, 0.6f, 0.0f},
                          draw_rect);
            } break;
            case UI_TOGGLE_AABB_TREE: {
                push_rect(&game_state->ui_render_group,
                          hovering ? color_t {0.6f, 0.2f, 0.2f} : color_t {0.6f, 0.0f, 0.0f},
                          draw_rect);
            } break;
            case UI_TOGGLE_PERFORMANCE: {
                push_rect(&game_state->ui_render_group,
                          hovering ? color_t {0.2f, 0.2f, 0.6f} : color_t {0.0f, 0.0f, 0.6f},
                          draw_rect);
            } break;
            case UI_COLOR_PICKER: {
                push_color_picker(&game_state->ui_render_group,
                                  item.element->color_picker.hsv,
                                  draw_rect);
            } break;
            default: {
                assert_(false);
            } break;
        }

        if (is_in_rect(mouse_position, draw_rect)) {
            hover_element = item.element;
        }
    }

    tools_state->hover_element = hover_element;
}

void tools_update_and_render(game_state_t* game_state,
                             tools_state_t* tools_state,
                             f32 dt,
                             window_description_t window,
                             game_input_t* game_input) {

    i32 wheel_delta = game_input->mouse.wheel_delta;
    tools_state->zoom_level = iclamp(tools_state->zoom_level + wheel_delta, -4, 4);

    m3x3 inv_ui_view = get_inverse_view_transform_3x3(game_state->ui_camera);
    v2 mouse_position = inv_ui_view * game_input->mouse.normalized_position;

    if (wheel_delta > 0) {
        f32 old_factor = game_state->main_camera.zoom.factor;
        f32 new_factor = get_factor(tools_state->zoom_level);
        v2 old_center = game_state->main_camera.zoom.relative_center;
        v2 target_center = v2 {
            (game_input->mouse.position.x / (f32)window.width) * 2.0f - 1.0f,
            (game_input->mouse.position.y / (f32)window.height) * -2.0f + 1.0f,
        };

        v2 new_center = (tools_state->zoom_level > 0) ?
        ((target_center - old_center) * (new_factor - old_factor) / new_factor) + old_center :
        v2 {0};
        game_state->main_camera.zoom.factor = new_factor;
        game_state->main_camera.zoom.relative_center = new_center;
    } else if (wheel_delta < 0) {
        f32 old_factor = game_state->main_camera.zoom.factor;
        f32 new_factor = get_factor(tools_state->zoom_level);
        v2 old_center = game_state->main_camera.zoom.relative_center;
        v2 new_center = (tools_state->zoom_level > 0) ?
            old_center * ((old_factor - new_factor) / (old_factor - 1.0f)) :
            v2 {0};
        game_state->main_camera.zoom.factor = new_factor;
        game_state->main_camera.zoom.relative_center = new_center;
    }

    draw_tools_menu(game_state, tools_state, window, game_input);

    if (was_pressed(game_input->mouse.left_click)) {
        if (tools_state->hover_element) {
            tools_state->active_element = tools_state->hover_element;

            switch (tools_state->active_element->type) {
                case UI_DRAGGABLE_WIDGET: {
                    tools_state->active_element->draggable_widget.initial_mouse_position =
                        mouse_position; 
                } break;
                case UI_COLOR_PICKER: {
                    v2 item_offset = tools_state->active_element->offset;
                    ui_element_t* parent = tools_state->active_element->parent;
                    while (parent) {
                        item_offset += parent->offset;
                        parent = parent->parent;
                    }
                    v2 relative_mouse = mouse_position - item_offset;

                    v2 normalized = v2 {
                        relative_mouse.x / tools_state->active_element->size.x,
                        relative_mouse.y / tools_state->active_element->size.y,
                    };

                    if (normalized.x > 0.9f) {
                        tools_state->active_element->color_picker.hsv.h = normalized.y;
                    } else {
                        tools_state->active_element->color_picker.hsv.s = normalized.x;
                        tools_state->active_element->color_picker.hsv.v = normalized.y;
                    }
                } break;
            }
        } else {
            if (tools_state->selected_render_item >= 0) {
                
            }
        }
    }

    if (was_released(game_input->mouse.left_click)) {
        if (tools_state->active_element &&
            tools_state->active_element == tools_state->hover_element) {
            switch (tools_state->active_element->type) {
                case UI_TOGGLE_MENU: {
                    tools_state->active_element->parent->menu.active =
                        !tools_state->active_element->parent->menu.active;
                } break;
                case UI_TOGGLE_WIREFRAMES: {
                    tools_state->debug_state.draw_wireframes =
                        !tools_state->debug_state.draw_wireframes;
                } break;
                case UI_TOGGLE_AABB_TREE: {
                    tools_state->debug_state.draw_aabb_tree =
                        !tools_state->debug_state.draw_aabb_tree;
                } break;
                case UI_TOGGLE_PERFORMANCE: {
                    tools_state->debug_state.show_performance =
                        !tools_state->debug_state.show_performance;
                } break;
                case UI_DRAGGABLE_WIDGET: {
                    v2 delta = mouse_position -
                        tools_state->active_element->draggable_widget.initial_mouse_position;
                    tools_state->active_element->offset =
                        tools_state->active_element->offset + delta;
                } break;
            }
        } else {
            glBindFramebuffer(GL_READ_FRAMEBUFFER,
                              game_state->main_render_group.frame_buffer.id);
            glReadBuffer(GL_COLOR_ATTACHMENT1);
            glReadPixels((i32)mouse_position.x,
                         (i32)mouse_position.y,
                         1,
                         1,
                         GL_RED_INTEGER,
                         GL_INT,
                         &tools_state->selected_render_item);
        }

        tools_state->active_element = 0;
    }

    if (tools_state->selected_render_item >= 0) {
        debug_push_ui_text_f(game_state,
                             tools_state,
                             window,
                             v2 {60.0f,60.0f},
                             RGBA_BLACK,
                             "%d",
                             tools_state->selected_render_item);
    }

    debug_update_and_render(game_state,
                            tools_state,
                            dt,
                            window,
                            game_input);
}

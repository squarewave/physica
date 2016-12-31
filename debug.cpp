#include <stdio.h>
#include <stdarg.h>

#include "typedefs.h"
#include "game.h"
#include "stb_truetype.h"

const f32 LINE_HEIGHT = 16.0f;

void process_debug_log(tools_state_* tools_state) {
    char* buffer = tools_state->debug_state.performance_log;

    for (int i = 0; i < max_debug_counter; ++i) {
        debug_block_ block = debug_blocks[i];

        i32 j = i;

        while(j > 0 && debug_blocks[j-1].total_cycles < block.total_cycles) {
            debug_blocks[j] = debug_blocks[j-1];
            j--;
        }

        debug_blocks[j] = block;
    }

    const i32 MAX_BARS = 20;
    f64 max_cycles = (f64)debug_blocks[0].total_cycles;

    for (int i = 0; i < max_debug_counter; ++i) {
        if (debug_blocks[i].id) {
            buffer += sprintf(buffer, "%-32s %14I64u cy,    %5d calls ",
                              debug_blocks[i].id,
                              debug_blocks[i].total_cycles,
                              debug_blocks[i].call_count);

            i32 bar_count = (i32)(((f64)debug_blocks[i].total_cycles / (f64)max_cycles)
                                        * (f64)MAX_BARS);
            for (int j = 0; j < bar_count; ++j) {
                *buffer = '|';
                buffer++;
            }

            *buffer = '\n';
            buffer++;

            assert_(buffer - tools_state->debug_state.performance_log <
                    (i64)ARRAY_SIZE(tools_state->debug_state.performance_log));

        }
    }

    for (int i = 0; i < max_debug_counter; ++i) {
        debug_blocks[i].id = 0;
        debug_blocks[i].total_cycles = 0;
        debug_blocks[i].call_count = 0;
    }


    *buffer = 0;
    buffer++;

    assert_(buffer - tools_state->debug_state.performance_log <
            (i64)ARRAY_SIZE(tools_state->debug_state.performance_log));
}

inline f32
debug_get_seconds_elapsed(u64 old, u64 current) {
    return ((f32)(current - old) / (f32)(SDL_GetPerformanceFrequency()));
}

void
debug_load_monospace_font(tools_state_* tools_state) {
    tools_state->debug_state.monospace_font =
        load_font("assets/SourceCodePro-Regular.ttf", LINE_HEIGHT);
}

void
debug_push_ui_text(game_state_* game_state,
                   tools_state_* tools_state,
                   window_description_ window,
                   v2 bottom_left,
                   rgba_ color,
                   char* text) {
    TIMED_FUNC();

    v2 original_pos = bottom_left;

    bottom_left.y = (f32)window.height - bottom_left.y - 1;

    font_spec_* font = &tools_state->debug_state.monospace_font;
    tex2 texture = font->texture;

    while (*text) {
        if (*text == '\n') {
            bottom_left.y += LINE_HEIGHT;
            bottom_left.x = original_pos.x;
        } else if (*text >= 32) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font->baked_chars,
                               font->texture.width,
                               font->texture.height,
                               *text-32,
                               &bottom_left.x,
                               &bottom_left.y,
                               &q,
                               1);

            f32 qt_width = (q.s1 - q.s0) * texture.width;
            f32 qt_height = (q.t1 - q.t0) * texture.height;

            rect_i source_rect;
            source_rect.min_x = (i32)(q.s0 * (f32)texture.width);
            source_rect.max_x = (i32)(q.s1 * (f32)texture.width);
            source_rect.max_y = (i32)((1.0f - q.t0) * (f32)texture.height);
            source_rect.min_y = (i32)((1.0f - q.t1) * (f32)texture.height);

            v2 min;
            v2 max;
            min.x = q.x0;
            max.x = q.x1;
            max.y = window.height - q.y0 - 1;
            min.y = window.height - q.y1 - 1;

            v2 center = 0.5f * (max + min);
            v2 hotspot = v2 {
                0.5f * qt_width,
                0.5f * qt_height,
            };

            push_texture(&game_state->ui_render_group,
                         center,
                         hotspot,
                         1.0f,
                         texture,
                         source_rect,
                         color,
                         0.0f,
                         0.0f);
        }
        ++text;
    }

    // f32 current_left = 0.0f;
    // while (*text) {
    //     i32 c = (i32)*text;

    //     glyph_spec_ glyph = game_state->debug_state.monospace_font[c - 32];

    //     tex2 texture = glyph.texture;
    //     v2 hotspot = v2{0};

    //     f32 width = (f32)texture.width;
    //     f32 height = (f32)texture.height;

    //     push_texture(&game_state->ui_render_group,
    //                  bottom_left + v2{ current_left + glyph.left_side_bearing,0.0f},
    //                  hotspot,
    //                  1.0f,
    //                  texture,
    //                  rect_i {0,0,texture.width,texture.height},
    //                  color,
    //                  0.0f,
    //                  0.0f);

    //     current_left += glyph.advance_width;
    //     text++;
    // }
}

void debug_push_ui_text_f(game_state_* game_state,
                          tools_state_* tools_state,
                          window_description_ window,
                          v2 bottom_left,
                          rgba_ color,
                          char* format,
                          ...)
{
    TIMED_FUNC();
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, ARRAY_SIZE(buffer), format, args);
    debug_push_ui_text(game_state, tools_state, window, bottom_left, color, buffer); 
    va_end(args);
}

void debug_draw_aabb_tree(game_state_* game_state) {
    phy_aabb_tree_* tree = &game_state->physics_state.aabb_tree;

    i32 stack[LARGE_STACK_SIZE] = {0};

    i32 stack_index = 0;
    stack[stack_index++] = tree->root;

    while (stack_index > 0) {
        assert_(stack_index < (i32)ARRAY_SIZE(stack));

        phy_aabb_tree_node_* node = tree->nodes.at(stack[--stack_index]);

        phy_aabb_ aabb = node->fat_aabb;
        v2 diagonal = aabb.max - aabb.min;
        v2 center = 0.5f * (aabb.max + aabb.min);

        color_ color = node->is_asleep ?
            color_ {0.5f,0.5f,0.5f} :
            color_ {0.9f, 0.4f, 0.2f};

        f32 z = node->is_asleep ? 0.01f : 0.0f;

        push_rect_outline(&game_state->main_render_group,
                          color,
                          center,
                          diagonal,
                          0.0f,
                          z);

        if (node->type != LEAF_NODE) {
            stack[stack_index++] = node->left;
            stack[stack_index++] = node->right;
        }
    }
}

void
debug_draw_hulls(game_state_* game_state) {
    phy_state_* physics = &game_state->physics_state;
    for (int i = 0; i < physics->bodies.size; ++i) {
        phy_body_* body = physics->bodies.try_get(i);
        if (!body) {
            continue;
        }

        for (int j = 0; j < body->hulls.count; ++j) {
            phy_hull_* hull = body->hulls.at(j);
            switch (hull->type) {
                case HULL_RECT: {
                    push_rect_outline(&game_state->main_render_group,
                              color_ {0.2f, 0.9f, 0.2f},
                              hull->position,
                              v2 {hull->width, hull->height},
                              hull->orientation,
                              0.0f);
                } break;
                case HULL_FILLET_RECT: {
                    push_rect_outline(&game_state->main_render_group,
                              color_ {0.2f, 0.9f, 0.2f},
                              hull->position,
                              v2 {hull->width, hull->height},
                              hull->orientation,
                              0.0f);
                    // f32 fillet = hull->fillet;
                    // f32 inner_width = hull->width / 2.0f - fillet;
                    // f32 inner_height = hull->height / 2.0f - fillet;
                    // m2x2 rotation = get_rotation_matrix(hull->orientation);
                    // push_circle(&game_state->main_render_group,
                    //             color_ {0.9f, 0.2f, 0.2f},
                    //             hull->position + rotation * v2 {inner_width, inner_height},
                    //             hull->fillet,
                    //             0.0f);
                    // push_circle(&game_state->main_render_group,
                    //             color_ {0.9f, 0.2f, 0.2f},
                    //             hull->position + rotation * v2 {inner_width, -inner_height},
                    //             hull->fillet,
                    //             0.0f);
                    // push_circle(&game_state->main_render_group,
                    //             color_ {0.9f, 0.2f, 0.2f},
                    //             hull->position + rotation * v2 {-inner_width, -inner_height},
                    //             hull->fillet,
                    //             0.0f);
                    // push_circle(&game_state->main_render_group,
                    //             color_ {0.9f, 0.2f, 0.2f},
                    //             hull->position + rotation * v2 {-inner_width, inner_height},
                    //             hull->fillet,
                    //             0.0f);
                } break;
            }
        }
    }
}

void debug_init(tools_state_* tools_state) {
    debug_load_monospace_font(tools_state);
}

void debug_update_and_render(game_state_* game_state,
                             tools_state_* tools_state,
                             f32 dt,
                             window_description_ window,
                             game_input_* game_input) {

    persist u64 last_counter = SDL_GetPerformanceCounter();
    f32 actual_dt = debug_get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter());
    last_counter = SDL_GetPerformanceCounter();

    if (tools_state->debug_state.show_performance) {
        rgba_ color;

        f32 fps = 1.0f / actual_dt;

        if (fps > (0.9f * (f32)FRAME_RATE)) {
            color = RGBA_GREEN;
        } else if (fps > (0.75f * (f32)FRAME_RATE)) {
            color = RGBA_ORANGE;
        } else {
            color = RGBA_RED;
        }
        
        debug_push_ui_text_f(game_state,
                             tools_state,
                             window,
                             v2 {8.0f, (f32)window.height - 20.0f},
                             color,
                             "%3.1f fps", (f64)fps);

        debug_push_ui_text(game_state,
                           tools_state,
                           window,
                           v2 {8.0f, (f32)window.height - 40.0f},
                           RGBA_BLACK,
                           tools_state->debug_state.performance_log);
    }

    if (tools_state->debug_state.draw_wireframes) {
        debug_draw_hulls(game_state);
        
        if (game_input->mouse.left_click.transition_count &&
            game_input->mouse.left_click.ended_down) {
            m3x3 inverse_view_transform = get_inverse_view_transform_3x3(game_state->main_camera);
            v2 world_position = inverse_view_transform * game_input->mouse.normalized_position;
            tools_state->debug_state.selected = pick_body(&game_state->physics_state,
                                                         world_position);
        }
        
        if (tools_state->debug_state.selected) {

            sim_entity_* entity =
                get_hash_item_value(&game_state->entity_map,
                                    tools_state->debug_state.selected->entity.id);

            push_circle(&game_state->main_render_group,
                        color_ {0.4f, 1.0f, 0.4f},
                        entity->body->position,
                        2.0f * VIRTUAL_PIXEL_SIZE,
                        0.0f,
                        0);


            debug_push_ui_text_f(game_state,
                                 tools_state,
                                 window,
                                 v2 {0.0f, 0.0f},
                                 RGBA_RED,
                                 "%ld", tools_state->debug_state.selected->entity.id);
        }
    }

    if (tools_state->debug_state.draw_aabb_tree) {
        debug_draw_aabb_tree(game_state);
    }
}

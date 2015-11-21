#include "game.h"
#include "stb_truetype.h"

void
debug_load_monospace_font(game_state_t* game_state) {

    // load printable ASCII
    for (u8 i = 32; i <= 127; ++i) {
        game_state->debug_state.monospace_font[i - 32] =
            load_glyph("C:/Windows/fonts/consola.ttf", i, 20.0f);
    }
}

void
debug_push_ui_text(game_state_t* game_state, v2 bottom_left, char* text) {

    f32 current_left = 0.0f;
    while (*text) {
        i32 c = (i32)*text;

        glyph_spec_t glyph = game_state->debug_state.monospace_font[c - 32];

        tex2 texture = glyph.texture;
        v2 hotspot = v2{0};

        f32 width = (f32)texture.width;
        f32 height = (f32)texture.height;

        push_texture(&game_state->ui_render_group,
                     bottom_left + v2{ current_left + glyph.left_side_bearing,0.0f},
                     hotspot,
                     1.0f,
                     texture,
                     rect_i {0,0,texture.width,texture.height},
                     rgba_t{0},
                     0.0f,
                     0.0f);

        current_left += glyph.advance_width;
        text++;
    }
}

void debug_draw_aabb_tree(game_state_t* game_state) {
    phy_aabb_tree_t* tree = &game_state->physics_state.aabb_tree;

    i32 stack[LARGE_STACK_SIZE] = {0};

    i32 stack_index = 0;
    stack[stack_index++] = tree->root;

    while (stack_index > 0) {
        assert(stack_index < ARRAY_SIZE(stack));

        phy_aabb_tree_node_t* node = tree->nodes.at(stack[--stack_index]);

        phy_aabb_t aabb = node->fat_aabb;
        v2 diagonal = aabb.max - aabb.min;
        v2 center = 0.5f * (aabb.max + aabb.min);

        color_t color = node->is_asleep ?
            color_t {0.5f,0.5f,0.5f} :
            color_t {0.9f, 0.4f, 0.2f};

        push_rect(&game_state->main_render_group,
                  color,
                  center,
                  diagonal,
                  0.0f,
                  0.0f,
                  RENDER_WIREFRAME);

        if (node->type != LEAF_NODE) {
            stack[stack_index++] = node->left;
            stack[stack_index++] = node->right;
        }
    }
}

void
debug_draw_hulls(game_state_t* game_state) {
    phy_state_t* physics = &game_state->physics_state;
    for (int i = 0; i < physics->bodies.size; ++i) {
        phy_body_t* body = physics->bodies.try_get(i);
        if (!body) {
            continue;
        }

        for (int j = 0; j < body->hulls.count; ++j) {
            phy_hull_t* hull = body->hulls.at(j);
            switch (hull->type) {
                case HULL_RECT: {
                    push_rect(&game_state->main_render_group,
                              color_t {0.2f, 0.9f, 0.2f},
                              hull->position,
                              v2 {hull->width, hull->height},
                              hull->orientation,
                              0.0f,
                              RENDER_WIREFRAME);
                } break;
                case HULL_FILLET_RECT: {
                    push_rect(&game_state->main_render_group,
                              color_t {0.2f, 0.9f, 0.2f},
                              hull->position,
                              v2 {hull->width, hull->height},
                              hull->orientation,
                              0.0f,
                              RENDER_WIREFRAME);
                    f32 fillet = hull->fillet;
                    f32 inner_width = hull->width / 2.0f - fillet;
                    f32 inner_height = hull->height / 2.0f - fillet;
                    m2x2 rotation = get_rotation_matrix(hull->orientation);
                    push_circle(&game_state->main_render_group,
                                color_t {0.9f, 0.2f, 0.2f},
                                hull->position + rotation * v2 {inner_width, inner_height},
                                hull->fillet,
                                0.0f,
                                RENDER_WIREFRAME);
                    push_circle(&game_state->main_render_group,
                                color_t {0.9f, 0.2f, 0.2f},
                                hull->position + rotation * v2 {inner_width, -inner_height},
                                hull->fillet,
                                0.0f,
                                RENDER_WIREFRAME);
                    push_circle(&game_state->main_render_group,
                                color_t {0.9f, 0.2f, 0.2f},
                                hull->position + rotation * v2 {-inner_width, -inner_height},
                                hull->fillet,
                                0.0f,
                                RENDER_WIREFRAME);
                    push_circle(&game_state->main_render_group,
                                color_t {0.9f, 0.2f, 0.2f},
                                hull->position + rotation * v2 {-inner_width, inner_height},
                                hull->fillet,
                                0.0f,
                                RENDER_WIREFRAME);
                } break;
            }
        }
    }
}

void debug_init(game_state_t* game_state) {
    debug_load_monospace_font(game_state);
}

void debug_update_and_render(game_state_t* game_state,
                             f32 dt,
                             game_input_t* game_input) {

    if (game_input->mouse.left_click.transition_count &&
        game_input->mouse.left_click.ended_down) {
        m3x3 inverse_view_transform = get_inverse_view_transform_3x3(game_state->main_camera);
        v2 world_position = inverse_view_transform * game_input->mouse.normalized_position;
        game_state->debug_state.selected = pick_body(&game_state->physics_state,
                                                     world_position);
    }
    
    if (game_state->debug_state.selected) {
        push_circle(&game_state->main_render_group,
                    color_t {0.4f, 1.0f, 0.4f},
                    game_state->debug_state.selected->position,
                    2.0f * VIRTUAL_PIXEL_SIZE,
                    0.0f,
                    0);

        char char_buffer[256];
        sprintf(char_buffer, "%lld", game_state->debug_state.selected->entity.id);
        debug_push_ui_text(game_state, v2 {0.0f, 0.0f}, char_buffer);
    }

    if (false) {
        debug_draw_hulls(game_state);
    }

    if (false) {
        debug_draw_aabb_tree(game_state);
    }
}
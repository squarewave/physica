 #include "game.h"

 void
 debug_draw_physics(game_state_t* game_state) {
    phy_state_t* physics = &game_state->physics_state;
    for (int i = 0; i < physics->bodies.size; ++i) {
        phy_body_t* body = physics->bodies.try_get(i);
        if (!body) {
            continue;
        }
        // push_circle(&game_state->main_render_group,
        //             color_t {0.4f, 1.0f, 0.4f},
        //             body->position,
        //             2.0f * VIRTUAL_PIXEL_SIZE,
        //             0.0f,
        //             0);
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
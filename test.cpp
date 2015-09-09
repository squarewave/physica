//
// Created by doug on 4/16/15.
//

#include <cfloat>
#include "game.h"
#include "game_render.h"
#include "physica.h"

void
initialize_game_state(game_state_t* game_state) {
    i64 world_memory_size = 1024 * 1024 * 64; // 64 megs
    i64 physics_memory_size = 1024 * 1024 * 64;
    i64 memory_layout[32] = {
        sizeof(game_state_t),
        world_memory_size,
        physics_memory_size
    };
    i64 memory_index = 0;
    u8* memory_location = (u8*)game_state;

    #define __MAKE_ARENA(arena) do {\
        arena.size = memory_layout[memory_index++];\
        memory_location += arena.size;\
        arena.base = memory_location;\
    } while (0)

    __MAKE_ARENA(game_state->world_arena);
    __MAKE_ARENA(game_state->physics_arena);

    game_state->entity_capacity = 4000;

    game_state->initialized = true;
}

struct physics_state_t {
    f32 x, y;
};

sim_entity_t*
make_block(game_state_t *game_state, f32 width, f32 height, f32 mass) {
    assert(game_state->entity_count < game_state->entity_capacity);
    sim_entity_t* result = game_state->entities + game_state->entity_count;
    game_state->entity_count++;

    result->width = width;
    result->height = height;
    result->body = phy_add_body(game_state->physics_arena);

    phy_body_t* body = result->body;
    body->mass = mass;
    body->inv_mass = 1.0f / body->mass;
    body->moment = 1.0f/12.0f * body->mass * (width * width + height * height);
    body->inv_moment = 1.0f / body->moment;
    body->hulls = phy_add_hulls(game_state->physics_arena, 1);
    body->aabb_node_index = -1;

    phy_hull_t * hull = body->hulls.values;
    hull->mass = mass;
    hull->inv_mass = 1.0f / hull->mass;
    hull->moment = 1.0f;
    hull->inv_moment = 1.0f/12.0f *
            hull->mass *
            (width * width + height * height);
    hull->points = phy_add_points(game_state->physics_arena, 4);

    hull->points.values[0] = {result->width / 2.0f, result->height / 2.0f};
    hull->points.values[1] = {-result->width / 2.0f, result->height / 2.0f};
    hull->points.values[2] = {-result->width / 2.0f, -result->height / 2.0f};
    hull->points.values[3] = {result->width / 2.0f, -result->height / 2.0f};
    return result;
}

i32
meters_to_pixels(f32 meters) {
    return meters * 30.0f;
}

void
game_update_and_render(game_state_t* game_state, f64 dt,
                            video_buffer_description_t buffer_description,
                            game_input_t game_input) {

//    dt /= 20.0f;
    if (!game_state->initialized) {
        initialize_game_state(game_state);

        game_state->camera.center = v2 {-20.0f, 70.0f};
        game_state->camera.to_top_left = v2 {
                START_WIDTH / 2, START_HEIGHT / 2};
        game_state->camera.scaling = 2.0f;

        phy_init(game_state->physics_arena);

        game_state->entities = (sim_entity_t*)game_state->world_arena.base;

        sim_entity_t* entity = make_block(game_state, 2.0f, 2.0f, 20.0f);
        entity->color = 0xddff55dd;
        entity->body->position = {0,3.5};
        entity->body->velocity = {0,0};
        entity->body->angular_velocity = 0.0f;

        // entity = make_block(game_state, 2.0f, 2.0f, 20.0f);
        // entity->color = 0xff55dddd;
        // entity->body->position = {0,4};
        // entity->body->velocity = {0,0};
        // entity->body->angular_velocity = -0.0f;
//
//        entity = make_block(game_state, 2.0f, 2.0f, 20.0f);
//        entity->color = 0xffdddd55;
//        entity->body->position = {-2,0};
//        entity->body->velocity = {0,0};
//        entity->body->angular_velocity = 0.0f;
//
//        entity = make_block(game_state, 2.0f, 2.0f, 20.0f);
//        entity->color = 0xffff8877;
//        entity->body->position = {-2,6};
//        entity->body->velocity = {0,0};
//        entity->body->angular_velocity = 0.0f;
//
//        entity = make_block(game_state, 2.0f, 2.0f, 20.0f);
//        entity->color = 0xff22aacc;
//        entity->body->position = {-2,8};
//        entity->body->velocity = {0,0};
//        entity->body->angular_velocity = 0.0f;
//
//        entity = make_block(game_state, 2.0f, 2.0f, 20.0f);
//        entity->color = 0xffaaffaa;
//        entity->body->position = {-2,10};
//        entity->body->velocity = {0,0};
//        entity->body->angular_velocity = 0.0f;

        entity = make_block(game_state, 20.0f, 10.0f, 1000.0f);
        entity->color = 0xffaaddcc;
        entity->body->position = {7,-5.5};
        entity->body->orientation = 0.0f;
        entity->body->velocity = {0,0};
        entity->body->flags = PHY_FIXED_FLAG;
        entity->body->inv_mass = 0;
        entity->body->inv_moment = 0;

//        float thing = 1.0;
//        for (f32 i = -6; i <= 6.1; i += thing) {
//            for (f32 j = -6; j < 6.1; j += thing) {
//                entity = make_block(game_state, 0.5f, 0.5f, 0.25f);
//                entity->body->position = {i,j};
//            }
//        }

        phy_set_gravity(game_state->physics_arena, v2 {0, -5});
    }

    draw_rectangle(buffer_description, from_rgb(v3 {0.3f, 0.35f, 0.3f}),
                   buffer_description.width / 2, buffer_description.height / 2,
                   buffer_description.width, buffer_description.height, 0.0f);

    phy_update(game_state->physics_arena, dt);

    camera_t camera = game_state->camera;
    m3x3 flip_y = identity_3x3();
    flip_y.r2.c2 = -1;
    flip_y.r2.c3 = camera.to_top_left.y * 2;
    m3x3 scale = get_scaling_matrix(camera.scaling) *
            get_scaling_matrix(30.0f);

    m3x3 camera_space_transform =
            flip_y *
            get_translation_matrix(camera.to_top_left) *
            get_translation_matrix(-camera.center) *
            scale;

    game_state->entities[0].body->force = v2 {0,0};
    game_state->entities[0].body->torque = 0.0f;
    for (int i = 0; i < game_state->entity_count; ++i) {
        sim_entity_t* entity = &game_state->entities[i];
        v2 camera_p = camera_space_transform *
                entity->body->position;

        v2 entity_box = scale * v2 {entity->width, entity->height};

        draw_rectangle(buffer_description, entity->color,
                       camera_p.x,
                       camera_p.y,
                       entity_box.x,
                       entity_box.y,
                       entity->body->orientation);
    }

    draw_debug_points(buffer_description, camera);

}

v2
do_support(phy_hull_t* hull, v2 direction) {
    f32 greatest = -FLT_MAX;
    f32 last = -FLT_MAX;
    m2x2 rotation = get_rotation_matrix(hull->orientation);
    v2 result = {};
    for (int i = 0; i < hull->points.count; ++i) {
        v2 transformed = rotation * hull->points[i] + hull->position;
        f32 test = dot(transformed, direction);
        if (test > greatest) {
            greatest = test;
            result = transformed;
        }
        last = test;
    }
    return result;
}

phy_support_result_t
do_support(phy_hull_t* a, phy_hull_t* b, v2 direction) {
    phy_support_result_t result;
    result.p_a = do_support(a, direction);
    result.p_b = do_support(b, -direction);
    result.p = result.p_a - result.p_b;
    return result;
}

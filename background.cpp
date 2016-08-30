#include "background.h"
#include "random.h"

const f32 MOTE_AREA_RADIUS_Y = 8.0f;
const f32 MOTE_AREA_RADIUS_X = 8.0f;
const f32 FLOATY_AREA_RADIUS_Y = 12.0f;
const f32 FLOATY_AREA_RADIUS_X = 12.0f;

const f32 MIN_MOTE_Z = 0.94f;
const f32 MAX_MOTE_Z = 0.96f;
const f32 MIN_FLOATY_Z = 0.90f;
const f32 MAX_FLOATY_Z = 0.94f;

const f32 MIN_DISTANCE_EFFECT = 0.4f;
const f32 MAX_DISTANCE_EFFECT = 0.6f;

const i32 MOTE_COUNT = 10000;
const i32 ATTRACTOR_COUNT = 0;

global u32 palette[] = {
    0xffd7eaf1, // 0: sky blue
    0xffd7eaf1, // 0: sky blue
    0xffd7eaf1, // 0: sky blue
    0xfff8fde3, // 1: cloud white
    0xfff8fde3, // 1: cloud white
    0xfff8fde3, // 1: cloud white
    0xfff8fde3, // 1: cloud white
    0xffbcc4a1, // 2: dark grass green
    0xffefdfb8, // 3: orange-ish
    0xffefdfb8, // 3: orange-ish
    0xffefdfb8, // 3: orange-ish
    0xffefdfb8, // 3: orange-ish
    0xffefdfb8, // 3: orange-ish
    0xffefdfb8, // 3: orange-ish
    0xffefdfb8, // 3: orange-ish
    0xffc3b9a3, // 4: brown
    0xffd4e3ec, // 5: light blue
    0xffd4e3ec, // 5: light blue
    0xffd4e3ec, // 5: light blue
    0xffd4e3ec, // 5: light blue
    0xffd4e3ec, // 5: light blue
    0xffd4e3ec, // 5: light blue
    0xffd4e3ec, // 5: light blue
    0xffd4e3ec, // 5: light blue
    0xffd7eaf1, // 0: sky blue
    0xffd7eaf1, // 0: sky blue
    0xffd7eaf1, // 0: sky blue
    0xfff8fde3, // 1: cloud white
    0xfff8fde3, // 1: cloud white
    0xfff8fde3, // 1: cloud white
    0xfff8fde3, // 1: cloud white
    0xffefdfb8, // 3: orange-ish
    0xffefdfb8, // 3: orange-ish
    0xffefdfb8, // 3: orange-ish
    0xffefdfb8, // 3: orange-ish
    0xffefdfb8, // 3: orange-ish
    0xffefdfb8, // 3: orange-ish
    0xffefdfb8, // 3: orange-ish
    0xffd4e3ec, // 5: light blue
    0xffd4e3ec, // 5: light blue
    0xffd4e3ec, // 5: light blue
    0xffd4e3ec, // 5: light blue
    0xffd4e3ec, // 5: light blue
    0xffd4e3ec, // 5: light blue
    0xffd4e3ec, // 5: light blue
    0xffd4e3ec, // 5: light blue
};

global char* texture_paths[] = {
    "assets/cloud.png",
    "assets/cloud_3.png",
    "assets/cloud_4.png",
    "assets/cloud_5.png",
    "assets/cloud_6.png",
    "assets/cloud_7.png",
    "assets/tower.png",
    "assets/castle.png"
};

void create_background(game_state_t* game_state,
                       background_t* background) {

    background->background_color = to_rgb(0xffc8dfec);
    background->motes.count = MOTE_COUNT;
    background->motes.values = PUSH_ARRAY(&game_state->render_arena,
                                          MOTE_COUNT,
                                          mote_t);

    background->attractors.count = ATTRACTOR_COUNT;
    background->attractors.values = PUSH_ARRAY(&game_state->render_arena,
                                          ATTRACTOR_COUNT,
                                          mote_attractor_t);
    const i32 floaty_count = 70;
    background->floaties.count = floaty_count;
    background->floaties.values = PUSH_ARRAY(&game_state->render_arena,
                                             floaty_count,
                                             floaty_t);

    const i32 texture_count = ARRAY_SIZE(texture_paths);
    background->textures.count = texture_count;
    background->textures.values = PUSH_ARRAY(&game_state->render_arena,
                                             texture_count,
                                             tex2);

    for (int i = 0; i < texture_count; ++i) {
        tex2 texture = load_image(texture_paths[i]);
        background->textures.set(i, texture);
    }

    for (int i = 0; i < floaty_count; ++i) {
        floaty_t* floaty = background->floaties.at(i);

        i32 texture_index = random_i32(0, texture_count);
        floaty->texture = background->textures[texture_index];
        floaty->source_rect = rect_i {0,0,floaty->texture.width, floaty->texture.height};

        floaty->z = random_f32(0.0f, 1.0f);
        // f32 z = scale(floaty->z, MIN_FLOATY_Z, MAX_FLOATY_Z);
        // f32 parallax = 1.0f - z;
        f32 min_x = -FLOATY_AREA_RADIUS_X;
        f32 max_x = FLOATY_AREA_RADIUS_X;
        f32 min_y = -FLOATY_AREA_RADIUS_Y;
        f32 max_y = FLOATY_AREA_RADIUS_Y;

        floaty->center = v2 {
            random_f32(min_x, max_x),
            random_f32(min_y, max_y)
        };

        floaty->velocity = vpixels_to_meters(random_f32(0.0f, 0.5f));
    }

    for (i32 i = 0; i < MOTE_COUNT; ++i) {
        mote_t* mote = background->motes.at(i);

        mote->width = 1;
        mote->color = palette[random_i32(0, ARRAY_SIZE(palette))];

        mote->z = random_f32(0.0f, 1.0f);
        // f32 z = scale(mote->z, MIN_MOTE_Z, MAX_MOTE_Z);
        // f32 parallax = 1.0f - z;
        f32 min_x = -MOTE_AREA_RADIUS_X;
        f32 max_x = MOTE_AREA_RADIUS_X;
        f32 min_y = -MOTE_AREA_RADIUS_Y;
        f32 max_y = MOTE_AREA_RADIUS_Y;

        mote->center = v2 {
            random_f32(min_x, max_x),
            random_f32(min_y, max_y)
        };
        mote->velocity = v2 {
            vpixels_to_meters(random_f32(-2.0f, 2.0f)),
            0.0f
        };
    }

    for (i32 i = 0; i < ATTRACTOR_COUNT; ++i) {
        mote_attractor_t* attractor = background->attractors.at(i);

        attractor->position = v2 {
            random_f32(-1.0f, 1.0f),
            random_f32(-1.0f, 1.0f)
        };
    }
}

void update_background(background_t* background,
                       render_group_t* render_group,
                       camera_t camera,
                       f32 dt) {
    TIMED_FUNC();

    // u32 u32_background_color = from_rgb(background->background_color);

    m3x3 inverse_view = get_inverse_view_transform_3x3(camera);

    for (i32 i = 0; i < background->floaties.count; ++i) {
        floaty_t* floaty = background->floaties.at(i);

        floaty->center.x += floaty->velocity * dt;

        f32 z = scale(floaty->z, MIN_FLOATY_Z, MAX_FLOATY_Z);
        f32 x = floaty->center.x;
        f32 y = floaty->center.y;
        f32 parallax = 1.0f - z;
        f32 min_x = (camera.center.x * parallax - FLOATY_AREA_RADIUS_X);
        f32 max_x = (camera.center.x * parallax + FLOATY_AREA_RADIUS_X);
        f32 min_y = (camera.center.y * parallax - FLOATY_AREA_RADIUS_Y);
        f32 max_y = (camera.center.y * parallax + FLOATY_AREA_RADIUS_Y);

        if (x < min_x || x > max_x) {
            x = wrap(x, min_x, max_x);
            y = random_f32(min_y, max_y);
            floaty->texture = background->textures[random_i32(0, background->textures.count)];   
            floaty->source_rect = rect_i {0,0,floaty->texture.width, floaty->texture.height};
        }

        y = snap_to(wrap(y, min_y, max_y), VIRTUAL_PIXEL_SIZE);

        floaty->center.x = x;
        floaty->center.y = y;

        push_texture(render_group,
                     v2 { snap_to(floaty->center.x, VIRTUAL_PIXEL_SIZE), floaty->center.y },
                     v2 { floaty->texture.width / 2.0f, floaty->texture.height / 2.0f },
                     VIRTUAL_PIXEL_SIZE,
                     floaty->texture,
                     floaty->source_rect,
                     rgba_t{0.7f, 0.7f, 0.75f,
                     scale(floaty->z, MIN_DISTANCE_EFFECT, MAX_DISTANCE_EFFECT)},
                     0.0f,
                     z,
                     true);
    }

    for (i32 i = 0; i < background->motes.count; ++i) {
        mote_t* mote = background->motes.at(i);

        f32 z = scale(mote->z, MIN_MOTE_Z, MAX_MOTE_Z);
        f32 parallax = 1.0f - z;

        mote->center += mote->velocity * dt;

        f32 x = mote->center.x;
        f32 y = mote->center.y;
        f32 min_x = (camera.center.x * parallax - MOTE_AREA_RADIUS_X);
        f32 max_x = (camera.center.x * parallax + MOTE_AREA_RADIUS_X);
        f32 min_y = (camera.center.y * parallax - MOTE_AREA_RADIUS_Y);
        f32 max_y = (camera.center.y * parallax + MOTE_AREA_RADIUS_Y);
        x = wrap(x, min_x, max_x);
        y = wrap(y, min_y, max_y);
        mote->center.x = x;
        mote->center.y = y;

        push_rect(render_group,
                  to_rgb(mote->color),
                  v2 { snap_to(mote->center.x, VIRTUAL_PIXEL_SIZE),
                    snap_to(mote->center.y, VIRTUAL_PIXEL_SIZE) },
                  v2 { (f32)mote->width, 1.0f } * VIRTUAL_PIXEL_SIZE,
                  0.0f,
                  z,
                  true);
    }

    for (int i = 0; i < background->attractors.count; ++i) {
        mote_attractor_t* attractor = background->attractors.at(i);
        v2 position = inverse_view * v2 { attractor->position.x, attractor->position.y };

        push_rect(render_group,
                  to_rgb(0xffff0000),
                  position,
                  v2 { 2.0f, 2.0f } * VIRTUAL_PIXEL_SIZE,
                  0.0f,
                  0.0f,
                  false);
    }
}

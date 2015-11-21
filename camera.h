#ifndef CAMERA_H
#define CAMERA_H

#include "physica_math.h"

struct camera_t {
    v2 center, to_top_right;
    f32 orientation;
};

inline m4x4
get_view_transform_4x4(camera_t camera) {
    f32 camera_scale_x = 1.0f / camera.to_top_right.x;
    f32 camera_scale_y = 1.0f / camera.to_top_right.y;
    m4x4 view_rotate = get_rotation_matrix_4x4(-camera.orientation);
    m4x4 view_translate = identity_4x4();
    m4x4 view_scale = identity_4x4();
    //scale
    view_scale.r1.c1 = camera_scale_x;
    view_scale.r2.c2 = camera_scale_y;
    //translate
    view_translate.r1.c4 = -camera.center.x;
    view_translate.r2.c4 = -camera.center.y;

    m4x4 view = view_scale * view_rotate * view_translate;
    return view;
}

inline m4x4
get_parallaxed_view_transform_4x4(camera_t camera, f32 parallax) {
    f32 camera_scale_x = 1.0f / camera.to_top_right.x;
    f32 camera_scale_y = 1.0f / camera.to_top_right.y;
    m4x4 view_rotate = get_rotation_matrix_4x4(-camera.orientation);
    m4x4 view_translate = identity_4x4();
    m4x4 view_scale = identity_4x4();
    //scale
    view_scale.r1.c1 = camera_scale_x;
    view_scale.r2.c2 = camera_scale_y;
    //translate
    view_translate.r1.c4 = -camera.center.x * parallax;
    view_translate.r2.c4 = -camera.center.y * parallax;

    m4x4 view = view_scale * view_rotate * view_translate;
    return view;
}

inline m3x3
get_inverse_view_transform_3x3(camera_t camera) {
    f32 camera_scale_x = camera.to_top_right.x;
    f32 camera_scale_y = camera.to_top_right.y;
    m3x3 view_rotate = get_rotation_matrix_3x3(camera.orientation);
    m3x3 view_translate = identity_3x3();
    m3x3 view_scale = identity_3x3();
    //scale
    view_scale.r1.c1 = camera_scale_x;
    view_scale.r2.c2 = camera_scale_y;
    //translate
    view_translate.r1.c3 = camera.center.x;
    view_translate.r2.c3 = camera.center.y;

    m3x3 view = view_translate * view_rotate * view_scale;
    return view;
}

#endif
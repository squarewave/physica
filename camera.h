#ifndef CAMERA_H
#define CAMERA_H

#include "physica_math.h"

struct camera_zoom_ {
    f32 factor;
    v2 relative_center;
};

struct camera_ {
    v2 center, to_top_right;
    f32 orientation;
    camera_zoom_ zoom;
};

inline m4x4
get_view_transform_4x4(camera_ camera) {
    assert_(camera.zoom.factor != 0.0f);
    v2 to_top_right = camera.to_top_right * (1.0f / camera.zoom.factor);
    v2 center = camera.center + rotate(v2 {
        camera.to_top_right.x * camera.zoom.relative_center.x,
        camera.to_top_right.y * camera.zoom.relative_center.y
    }, camera.orientation);

    f32 camera_scale_x = 1.0f / to_top_right.x;
    f32 camera_scale_y = 1.0f / to_top_right.y;
    m4x4 view_rotate = get_rotation_matrix_4x4(-camera.orientation);
    m4x4 view_translate = identity_4x4();
    m4x4 view_scale = identity_4x4();
    //scale
    view_scale.r1.c1 = camera_scale_x;
    view_scale.r2.c2 = camera_scale_y;
    //translate
    view_translate.r1.c4 = -center.x;
    view_translate.r2.c4 = -center.y;

    m4x4 view = view_scale * view_rotate * view_translate;
    return view;
}

inline m4x4
get_parallaxed_view_transform_4x4(camera_ camera, f32 parallax) {
    assert_(camera.zoom.factor != 0.0f);
    v2 to_top_right = camera.to_top_right * (1.0f / camera.zoom.factor);
    v2 center = camera.center * parallax + rotate(v2 {
        camera.to_top_right.x * camera.zoom.relative_center.x,
        camera.to_top_right.y * camera.zoom.relative_center.y
    }, camera.orientation);

    f32 camera_scale_x = 1.0f / to_top_right.x;
    f32 camera_scale_y = 1.0f / to_top_right.y;
    m4x4 view_rotate = get_rotation_matrix_4x4(-camera.orientation);
    m4x4 view_translate = identity_4x4();
    m4x4 view_scale = identity_4x4();
    //scale
    view_scale.r1.c1 = camera_scale_x;
    view_scale.r2.c2 = camera_scale_y;
    //translate
    view_translate.r1.c4 = -center.x;
    view_translate.r2.c4 = -center.y;

    m4x4 view = view_scale * view_rotate * view_translate;
    return view;
}

inline m3x3
get_parallaxed_view_transform_3x3(camera_ camera, f32 parallax) {
    assert_(camera.zoom.factor != 0.0f);
    v2 to_top_right = camera.to_top_right * (1.0f / camera.zoom.factor);
    v2 center = camera.center * parallax + rotate(v2 {
        camera.to_top_right.x * camera.zoom.relative_center.x,
        camera.to_top_right.y * camera.zoom.relative_center.y
    }, camera.orientation);

    f32 camera_scale_x = 1.0f / to_top_right.x;
    f32 camera_scale_y = 1.0f / to_top_right.y;
    m3x3 view_rotate = get_rotation_matrix_3x3(-camera.orientation);
    m3x3 view_translate = identity_3x3();
    m3x3 view_scale = identity_3x3();
    //scale
    view_scale.r1.c1 = camera_scale_x;
    view_scale.r2.c2 = camera_scale_y;
    //translate
    view_translate.r1.c3 = -center.x;
    view_translate.r2.c3 = -center.y;

    m3x3 view = view_scale * view_rotate * view_translate;
    return view;
}

inline m3x3
get_inverse_parallaxed_view_transform_3x3(camera_ camera, f32 parallax) {
    assert_(camera.zoom.factor != 0.0f);
    v2 to_top_right = camera.to_top_right * (1.0f / camera.zoom.factor);
    v2 center = (camera.center - rotate(v2 {
        camera.to_top_right.x * camera.zoom.relative_center.x,
        camera.to_top_right.y * camera.zoom.relative_center.y
    }, -camera.orientation)) * (1.0f / parallax);

    f32 camera_scale_x = to_top_right.x;
    f32 camera_scale_y = to_top_right.y;
    m3x3 view_rotate = get_rotation_matrix_3x3(camera.orientation);
    m3x3 view_translate = identity_3x3();
    m3x3 view_scale = identity_3x3();
    //scale
    view_scale.r1.c1 = camera_scale_x;
    view_scale.r2.c2 = camera_scale_y;
    //translate
    view_translate.r1.c3 = center.x;
    view_translate.r2.c3 = center.y;

    m3x3 view = view_translate * view_rotate * view_scale;
    return view;
}

inline m3x3
get_view_transform_3x3(camera_ camera) {
    assert_(camera.zoom.factor != 0.0f);
    v2 to_top_right = camera.to_top_right * (1.0f / camera.zoom.factor);
    v2 center = camera.center + rotate(v2 {
        camera.to_top_right.x * camera.zoom.relative_center.x,
        camera.to_top_right.y * camera.zoom.relative_center.y
    }, camera.orientation);

    f32 camera_scale_x = 1.0f / to_top_right.x;
    f32 camera_scale_y = 1.0f / to_top_right.y;
    m3x3 view_rotate = get_rotation_matrix_3x3(-camera.orientation);
    m3x3 view_translate = identity_3x3();
    m3x3 view_scale = identity_3x3();
    //scale
    view_scale.r1.c1 = camera_scale_x;
    view_scale.r2.c2 = camera_scale_y;
    //translate
    view_translate.r1.c3 = -center.x;
    view_translate.r2.c3 = -center.y;

    m3x3 view = view_scale * view_rotate * view_translate;
    return view;
}

inline m3x3
get_inverse_view_transform_3x3(camera_ camera) {
    assert_(camera.zoom.factor != 0.0f);
    v2 to_top_right = camera.to_top_right * (1.0f / camera.zoom.factor);
    v2 center = camera.center + rotate(v2 {
        camera.to_top_right.x * camera.zoom.relative_center.x,
        camera.to_top_right.y * camera.zoom.relative_center.y
    }, -camera.orientation);

    f32 camera_scale_x = to_top_right.x;
    f32 camera_scale_y = to_top_right.y;
    m3x3 view_rotate = get_rotation_matrix_3x3(camera.orientation);
    m3x3 view_translate = identity_3x3();
    m3x3 view_scale = identity_3x3();
    //scale
    view_scale.r1.c1 = camera_scale_x;
    view_scale.r2.c2 = camera_scale_y;
    //translate
    view_translate.r1.c3 = center.x;
    view_translate.r2.c3 = center.y;

    m3x3 view = view_translate * view_rotate * view_scale;
    return view;
}

inline m2x2
get_inverse_view_transform_2x2(camera_ camera) {
    assert_(camera.zoom.factor != 0.0f);
    v2 to_top_right = camera.to_top_right * (1.0f / camera.zoom.factor);
    v2 center = camera.center + rotate(v2 {
        camera.to_top_right.x * camera.zoom.relative_center.x,
        camera.to_top_right.y * camera.zoom.relative_center.y
    }, -camera.orientation);

    f32 camera_scale_x = to_top_right.x;
    f32 camera_scale_y = to_top_right.y;
    m2x2 view_rotate = get_rotation_matrix(camera.orientation);
    m2x2 view_scale = identity_2x2();
    //scale
    view_scale.r1.c1 = camera_scale_x;
    view_scale.r2.c2 = camera_scale_y;

    m2x2 view = view_rotate * view_scale;
    return view;
}

#endif

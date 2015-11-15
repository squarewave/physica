#ifndef ANIMATIONS_H_
#define ANIMATIONS_H_
#include "animations/may_walking_right.h"
#include "animations/may_walking_left.h"
#include "animations/may_standing_right.h"
#include "animations/may_standing_left.h"
#include "animations/martin_tie_adjust.h"
#include "animations/martin_standing_right.h"
#include "animations/martin_standing_left.h"
#include "animations/martin_running_right.h"
#include "animations/martin_running_left.h"
#include "animations/martin_jumping_right.h"
#include "animations/martin_jumping_left.h"

struct animations_list_t {
    animation_spec_t may_walking_right;
    animation_spec_t may_walking_left;
    animation_spec_t may_standing_right;
    animation_spec_t may_standing_left;
    animation_spec_t martin_tie_adjust;
    animation_spec_t martin_standing_right;
    animation_spec_t martin_standing_left;
    animation_spec_t martin_running_right;
    animation_spec_t martin_running_left;
    animation_spec_t martin_jumping_right;
    animation_spec_t martin_jumping_left;
};

animations_list_t load_animations(memory_arena_t* arena) {
    animations_list_t animations = {0};
    {
    animation_builder_t builder = may_walking_right_builder();
    animation_spec_t* spec = &animations.may_walking_right;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, builder.frame_count, animation_frame_t);
    tex2 texture = load_bmp(builder.bmp_filepath, 1);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_t frame = {0};
        frame.duration = builder.frame_duration;
        frame.texture = texture;
        frame.source_rect.min_y = 0;
        frame.source_rect.max_y = frame_height;
        frame.source_rect.min_x = i * frame_width;
        frame.source_rect.max_x = (i+1) * frame_width;
        frame.pixel_size = VIRTUAL_PIXEL_SIZE;
        frame.orientation = 0.0f;
        frame.hotspot = hotspot;

        spec->frames.set(i, frame);
    }
    }
    {
    animation_builder_t builder = may_walking_left_builder();
    animation_spec_t* spec = &animations.may_walking_left;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, builder.frame_count, animation_frame_t);
    tex2 texture = load_bmp(builder.bmp_filepath, 1);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_t frame = {0};
        frame.duration = builder.frame_duration;
        frame.texture = texture;
        frame.source_rect.min_y = 0;
        frame.source_rect.max_y = frame_height;
        frame.source_rect.min_x = i * frame_width;
        frame.source_rect.max_x = (i+1) * frame_width;
        frame.pixel_size = VIRTUAL_PIXEL_SIZE;
        frame.orientation = 0.0f;
        frame.hotspot = hotspot;

        spec->frames.set(i, frame);
    }
    }
    {
    animation_builder_t builder = may_standing_right_builder();
    animation_spec_t* spec = &animations.may_standing_right;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, builder.frame_count, animation_frame_t);
    tex2 texture = load_bmp(builder.bmp_filepath, 1);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_t frame = {0};
        frame.duration = builder.frame_duration;
        frame.texture = texture;
        frame.source_rect.min_y = 0;
        frame.source_rect.max_y = frame_height;
        frame.source_rect.min_x = i * frame_width;
        frame.source_rect.max_x = (i+1) * frame_width;
        frame.pixel_size = VIRTUAL_PIXEL_SIZE;
        frame.orientation = 0.0f;
        frame.hotspot = hotspot;

        spec->frames.set(i, frame);
    }
    }
    {
    animation_builder_t builder = may_standing_left_builder();
    animation_spec_t* spec = &animations.may_standing_left;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, builder.frame_count, animation_frame_t);
    tex2 texture = load_bmp(builder.bmp_filepath, 1);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_t frame = {0};
        frame.duration = builder.frame_duration;
        frame.texture = texture;
        frame.source_rect.min_y = 0;
        frame.source_rect.max_y = frame_height;
        frame.source_rect.min_x = i * frame_width;
        frame.source_rect.max_x = (i+1) * frame_width;
        frame.pixel_size = VIRTUAL_PIXEL_SIZE;
        frame.orientation = 0.0f;
        frame.hotspot = hotspot;

        spec->frames.set(i, frame);
    }
    }
    {
    animation_builder_t builder = martin_tie_adjust_builder();
    animation_spec_t* spec = &animations.martin_tie_adjust;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, builder.frame_count, animation_frame_t);
    tex2 texture = load_bmp(builder.bmp_filepath, 1);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_t frame = {0};
        frame.duration = builder.frame_duration;
        frame.texture = texture;
        frame.source_rect.min_y = 0;
        frame.source_rect.max_y = frame_height;
        frame.source_rect.min_x = i * frame_width;
        frame.source_rect.max_x = (i+1) * frame_width;
        frame.pixel_size = VIRTUAL_PIXEL_SIZE;
        frame.orientation = 0.0f;
        frame.hotspot = hotspot;

        spec->frames.set(i, frame);
    }
    }
    {
    animation_builder_t builder = martin_standing_right_builder();
    animation_spec_t* spec = &animations.martin_standing_right;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, builder.frame_count, animation_frame_t);
    tex2 texture = load_bmp(builder.bmp_filepath, 1);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_t frame = {0};
        frame.duration = builder.frame_duration;
        frame.texture = texture;
        frame.source_rect.min_y = 0;
        frame.source_rect.max_y = frame_height;
        frame.source_rect.min_x = i * frame_width;
        frame.source_rect.max_x = (i+1) * frame_width;
        frame.pixel_size = VIRTUAL_PIXEL_SIZE;
        frame.orientation = 0.0f;
        frame.hotspot = hotspot;

        spec->frames.set(i, frame);
    }
    }
    {
    animation_builder_t builder = martin_standing_left_builder();
    animation_spec_t* spec = &animations.martin_standing_left;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, builder.frame_count, animation_frame_t);
    tex2 texture = load_bmp(builder.bmp_filepath, 1);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_t frame = {0};
        frame.duration = builder.frame_duration;
        frame.texture = texture;
        frame.source_rect.min_y = 0;
        frame.source_rect.max_y = frame_height;
        frame.source_rect.min_x = i * frame_width;
        frame.source_rect.max_x = (i+1) * frame_width;
        frame.pixel_size = VIRTUAL_PIXEL_SIZE;
        frame.orientation = 0.0f;
        frame.hotspot = hotspot;

        spec->frames.set(i, frame);
    }
    }
    {
    animation_builder_t builder = martin_running_right_builder();
    animation_spec_t* spec = &animations.martin_running_right;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, builder.frame_count, animation_frame_t);
    tex2 texture = load_bmp(builder.bmp_filepath, 1);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_t frame = {0};
        frame.duration = builder.frame_duration;
        frame.texture = texture;
        frame.source_rect.min_y = 0;
        frame.source_rect.max_y = frame_height;
        frame.source_rect.min_x = i * frame_width;
        frame.source_rect.max_x = (i+1) * frame_width;
        frame.pixel_size = VIRTUAL_PIXEL_SIZE;
        frame.orientation = 0.0f;
        frame.hotspot = hotspot;

        spec->frames.set(i, frame);
    }
    }
    {
    animation_builder_t builder = martin_running_left_builder();
    animation_spec_t* spec = &animations.martin_running_left;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, builder.frame_count, animation_frame_t);
    tex2 texture = load_bmp(builder.bmp_filepath, 1);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_t frame = {0};
        frame.duration = builder.frame_duration;
        frame.texture = texture;
        frame.source_rect.min_y = 0;
        frame.source_rect.max_y = frame_height;
        frame.source_rect.min_x = i * frame_width;
        frame.source_rect.max_x = (i+1) * frame_width;
        frame.pixel_size = VIRTUAL_PIXEL_SIZE;
        frame.orientation = 0.0f;
        frame.hotspot = hotspot;

        spec->frames.set(i, frame);
    }
    }
    {
    animation_builder_t builder = martin_jumping_right_builder();
    animation_spec_t* spec = &animations.martin_jumping_right;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, builder.frame_count, animation_frame_t);
    tex2 texture = load_bmp(builder.bmp_filepath, 1);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_t frame = {0};
        frame.duration = builder.frame_duration;
        frame.texture = texture;
        frame.source_rect.min_y = 0;
        frame.source_rect.max_y = frame_height;
        frame.source_rect.min_x = i * frame_width;
        frame.source_rect.max_x = (i+1) * frame_width;
        frame.pixel_size = VIRTUAL_PIXEL_SIZE;
        frame.orientation = 0.0f;
        frame.hotspot = hotspot;

        spec->frames.set(i, frame);
    }
    }
    {
    animation_builder_t builder = martin_jumping_left_builder();
    animation_spec_t* spec = &animations.martin_jumping_left;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, builder.frame_count, animation_frame_t);
    tex2 texture = load_bmp(builder.bmp_filepath, 1);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_t frame = {0};
        frame.duration = builder.frame_duration;
        frame.texture = texture;
        frame.source_rect.min_y = 0;
        frame.source_rect.max_y = frame_height;
        frame.source_rect.min_x = i * frame_width;
        frame.source_rect.max_x = (i+1) * frame_width;
        frame.pixel_size = VIRTUAL_PIXEL_SIZE;
        frame.orientation = 0.0f;
        frame.hotspot = hotspot;

        spec->frames.set(i, frame);
    }
    }
    return animations;
}
#endif
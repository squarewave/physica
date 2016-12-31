#ifndef ANIMATIONS_H_
#define ANIMATIONS_H_
#include "animations/may_walking_right.h"
#include "animations/may_walking_left.h"
#include "animations/may_standing_right.h"
#include "animations/may_standing_left.h"
#include "animations/may_running_right.h"
#include "animations/may_running_left.h"
#include "animations/may_jumping_right.h"
#include "animations/may_jumping_left.h"
#include "animations/martin_tie_adjust.h"
#include "animations/martin_standing_right.h"
#include "animations/martin_standing_left.h"
#include "animations/martin_running_right.h"
#include "animations/martin_running_left.h"
#include "animations/martin_jumping_right.h"
#include "animations/martin_jumping_left.h"
#include "animations/lilguy_standing_right.h"
#include "animations/lilguy_standing_left.h"
#include "animations/lilguy_running_right.h"
#include "animations/lilguy_running_left.h"
#include "animations/lilguy_mayor_standing_right.h"
#include "animations/lilguy_mayor_standing_left.h"
#include "animations/lilguy_mayor_running_right.h"
#include "animations/lilguy_mayor_running_left.h"

struct animations_list_ {
    animation_spec_ may_walking_right;
    animation_spec_ may_walking_left;
    animation_spec_ may_standing_right;
    animation_spec_ may_standing_left;
    animation_spec_ may_running_right;
    animation_spec_ may_running_left;
    animation_spec_ may_jumping_right;
    animation_spec_ may_jumping_left;
    animation_spec_ martin_tie_adjust;
    animation_spec_ martin_standing_right;
    animation_spec_ martin_standing_left;
    animation_spec_ martin_running_right;
    animation_spec_ martin_running_left;
    animation_spec_ martin_jumping_right;
    animation_spec_ martin_jumping_left;
    animation_spec_ lilguy_standing_right;
    animation_spec_ lilguy_standing_left;
    animation_spec_ lilguy_running_right;
    animation_spec_ lilguy_running_left;
    animation_spec_ lilguy_mayor_standing_right;
    animation_spec_ lilguy_mayor_standing_left;
    animation_spec_ lilguy_mayor_running_right;
    animation_spec_ lilguy_mayor_running_left;
};

animations_list_ load_animations(memory_arena_* arena) {
    animations_list_ animations = {0};
    {
    animation_builder_ builder = may_walking_right_builder();
    animation_spec_* spec = &animations.may_walking_right;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = may_walking_left_builder();
    animation_spec_* spec = &animations.may_walking_left;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = may_standing_right_builder();
    animation_spec_* spec = &animations.may_standing_right;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = may_standing_left_builder();
    animation_spec_* spec = &animations.may_standing_left;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = may_running_right_builder();
    animation_spec_* spec = &animations.may_running_right;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = may_running_left_builder();
    animation_spec_* spec = &animations.may_running_left;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = may_jumping_right_builder();
    animation_spec_* spec = &animations.may_jumping_right;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = may_jumping_left_builder();
    animation_spec_* spec = &animations.may_jumping_left;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = martin_tie_adjust_builder();
    animation_spec_* spec = &animations.martin_tie_adjust;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = martin_standing_right_builder();
    animation_spec_* spec = &animations.martin_standing_right;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = martin_standing_left_builder();
    animation_spec_* spec = &animations.martin_standing_left;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = martin_running_right_builder();
    animation_spec_* spec = &animations.martin_running_right;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = martin_running_left_builder();
    animation_spec_* spec = &animations.martin_running_left;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = martin_jumping_right_builder();
    animation_spec_* spec = &animations.martin_jumping_right;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = martin_jumping_left_builder();
    animation_spec_* spec = &animations.martin_jumping_left;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = lilguy_standing_right_builder();
    animation_spec_* spec = &animations.lilguy_standing_right;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = lilguy_standing_left_builder();
    animation_spec_* spec = &animations.lilguy_standing_left;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = lilguy_running_right_builder();
    animation_spec_* spec = &animations.lilguy_running_right;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = lilguy_running_left_builder();
    animation_spec_* spec = &animations.lilguy_running_left;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = lilguy_mayor_standing_right_builder();
    animation_spec_* spec = &animations.lilguy_mayor_standing_right;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = lilguy_mayor_standing_left_builder();
    animation_spec_* spec = &animations.lilguy_mayor_standing_left;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = lilguy_mayor_running_right_builder();
    animation_spec_* spec = &animations.lilguy_mayor_running_right;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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
    animation_builder_ builder = lilguy_mayor_running_left_builder();
    animation_spec_* spec = &animations.lilguy_mayor_running_left;
    spec->cycle_point = builder.cycle_point;
    spec->frames.count = builder.frame_count;
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_);
    tex2 texture = load_image(builder.bmp_filepath);
    i32 frame_height = texture.height;
    assert_(texture.width % builder.frame_count == 0);
    i32 frame_width = texture.width / builder.frame_count;
    v2 hotspot = builder.hotspot;

    for (i32 i = 0; i < builder.frame_count; ++i) {
        animation_frame_ frame = {0};
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

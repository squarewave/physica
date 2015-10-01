//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_GAME_RENDER_H
#define PHYSICA_GAME_RENDER_H

#include "typedefs.h"
#include "physica_math.h"

struct tex2 {
    u32 width, height;
    u32* pixels;
};

struct video_buffer_description_t {
    void* memory;
    u32 width, height, pitch, bytes_per_pixel;
};

struct camera_t {
    v2 center, to_top_left;
    f32 scaling;
};

typedef v3 color_t;

const u32 RENDER_TYPE_RECT = 1;

struct render_rect_t {
  u32 type;
  v2 center;
  v2 diagonal;
  f32 orientation;
  color_t color;
};

struct render_object_t {
  union {
    u32 type;
    render_rect_t render_rect;
  };
};

struct render_group_t {
  vec<render_object_t> objects;
};

struct render_task_t {
  video_buffer_description_t buffer;
  camera_t camera;
  render_group_t* render_group;
  rect_i clip_rect;
};

void push_rect(render_group_t render_group, color_t color, v2 center, v2 diagonal, f32 orientation);

void draw_render_group(video_buffer_description_t buffer,
                       camera_t camera,
                       render_group_t render_group);

void draw_bmp(tex2 dest,
              tex2 source,
              i32 dest_hotspot_x, i32 dest_hotspot_y,
              f32 source_pixel_size,
              u32 source_left, u32 source_top,
              u32 source_width, u32 source_height,
              f32 source_hotspot_x, f32 source_hotspot_y, f32 orientation);

void draw_bmp(video_buffer_description_t buffer,
              tex2 bitmap,
              i32 dest_hotspot_x, i32 dest_hotspot_y,
              f32 source_pixel_size,
              u32 source_left, u32 source_top,
              u32 source_width, u32 source_height,
              f32 source_hotspot_x, f32 source_hotspot_y, f32 orientation);

void draw_rectangle(video_buffer_description_t buffer,
                    rect_i clip_rect,
                    color_t color, i32 center_x, i32 center_y, u32 width, u32 height,
                    f32 orientation = 0.0f);


void draw_rectangle(video_buffer_description_t buffer,
                    color_t color, i32 center_x, i32 center_y, u32 width, u32 height,
                    f32 orientation = 0.0f);

void add_debug_point(v2 point, i32 color);
void draw_debug_points();

#endif //PHYSICA_GAME_RENDER_H

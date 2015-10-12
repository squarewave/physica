//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_GAME_RENDER_H
#define PHYSICA_GAME_RENDER_H

#include "typedefs.h"
#include "physica_math.h"

struct tex2 {
    i32 width, height, pitch;
    u32* pixels;
};

struct video_buffer_description_t {
    void* memory;
    u32 width, height, pitch, bytes_per_pixel;
};

struct camera_t {
    v2 center, to_top_left;
    f32 pixels_per_meter;
    f32 scaling;
};

typedef v3 color_t;
typedef v4 rgba_t;

const u32 RENDER_TYPE_RECT = 1;
const u32 RENDER_TYPE_TEXTURE = 2;

struct render_rect_t {
  u32 type;
  v2 center;
  v2 diagonal;
  f32 orientation;
  color_t color;
};

struct render_texture_t {
  u32 type;
  tex2 texture;
  v2 center;
  v2 hotspot;
  f32 pixel_size;
  f32 orientation;
};

struct render_object_t {
  union {
    u32 type;
    render_rect_t render_rect;
    render_texture_t render_texture;
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

void push_rect(render_group_t render_group,
               color_t color,
               v2 center,
               v2 diagonal,
               f32 orientation);

render_object_t* push_texture(render_group_t* render_group,
                  v2 center,
                  v2 hotspot,
                  f32 pixel_size,
                  tex2 texture,
                  f32 orientation);

void draw_render_group(video_buffer_description_t buffer,
                       camera_t camera,
                       render_group_t render_group);


void draw_bmp(video_buffer_description_t buffer,
              rect_i clip_rect,
              tex2 bitmap,
              v2 center,
              f32 source_pixel_size,
              v2 hotspot,
              f32 orientation);

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

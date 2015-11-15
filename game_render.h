//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_GAME_RENDER_H
#define PHYSICA_GAME_RENDER_H

#include "typedefs.h"
#include "physica_math.h"

struct tex2 {
    i32 width, height, pitch;
    u32 texture_id;
    u32* pixels;
};

struct video_buffer_description_t {
    void* memory;
    u32 width, height, pitch, bytes_per_pixel;
};

struct camera_t {
    v2 center, to_top_left;
    f32 orientation;
};

typedef v3 color_t;
typedef v4 rgba_t;

enum render_type_T {
  RENDER_TYPE_RECT = 1,
  RENDER_TYPE_TEXTURE = 2,
  RENDER_TYPE_CIRCLE = 3,
};

enum render_flags_t {
  RENDER_WIREFRAME = 0x1,
};

struct render_rect_t {
  u32 type;
  u32 flags;
  v2 center;
  v2 diagonal;
  f32 orientation;
  color_t color;
};

struct render_texture_t {
  u32 type;
  u32 flags;
  tex2 texture;
  v2 center;
  v2 hotspot;
  rect_i source_rect;
  f32 pixel_size;
  f32 orientation;
};

struct render_circle_t {
  u32 type;
  u32 flags;
  v2 center;
  f32 radius;
  color_t color;
};

struct render_object_t {
  f32 z;
  union {
    struct {
      u32 type;
      u32 flags; 
    };
    render_rect_t render_rect;
    render_texture_t render_texture;
    render_circle_t render_circle;
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

#pragma pack(push, 1)
struct bitmap_header_t {
    u16 file_type;
    u32 file_size;
    u16 reserved_1;
    u16 reserved_2;
    u32 bitmap_offset;
    u32 size;
    i32 width;
    i32 height;
    u16 planes;
    u16 bits_per_pixel;
    u32 compression;
    u32 size_of_bitmap;
    i32 horz_resolution;
    i32 vert_resolution;
    u32 colors_used;
    u32 colors_important;
    u32 red_mask;
    u32 green_mask;
    u32 blue_mask;
};
#pragma pack(pop)

tex2 load_bmp(char* filename, i32 scaling);

render_object_t* push_rect(render_group_t* render_group,
                           color_t color,
                           v2 center,
                           v2 diagonal,
                           f32 orientation,
                           f32 z);

render_object_t* push_background(render_group_t* render_group,
                                 color_t color,
                                 video_buffer_description_t buffer);

render_object_t* push_texture(render_group_t* render_group,
                              v2 center,
                              v2 hotspot,
                              f32 pixel_size,
                              tex2 texture,
                              rect_i source_rect,
                              f32 orientation,
                              f32 z);

struct platform_services_t;

void draw_render_group(platform_services_t platform,
                       video_buffer_description_t buffer,
                       camera_t camera,
                       render_group_t* render_group);

void clear_render_group(render_group_t render_group);

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

void draw_circ_outline(video_buffer_description_t buffer,
                       rect_i clip_rect,
                       color_t color,
                       v2 center,
                       f32 radius);

void draw_rectangle(video_buffer_description_t buffer,
                    rect_i clip_rect,
                    color_t color, i32 center_x, i32 center_y, u32 width, u32 height,
                    f32 orientation = 0.0f);


void draw_rectangle(video_buffer_description_t buffer,
                    color_t color, i32 center_x, i32 center_y, u32 width, u32 height,
                    f32 orientation = 0.0f);

void add_debug_point(v2 point, i32 color);
void draw_debug_points();
void present_seconary_buffer();

#endif //PHYSICA_GAME_RENDER_H

//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_GAME_RENDER_H
#define PHYSICA_GAME_RENDER_H

#include "typedefs.h"
#include "physica_math.h"
#include "camera.h"
#include "stb_truetype.h"

struct tex2 {
    i32 width, height, pitch;
    u32 texture_id;
    u32* pixels;
};

struct glyph_spec_t {
    tex2 texture;
    v2 hotspot;
    f32 advance_width;
    f32 left_side_bearing;
};

struct font_spec_t {
    tex2 texture;
    stbtt_bakedchar baked_chars[96];
};

struct window_description_t {
    u32 width, height;
};

typedef v3 color_t;
typedef v4 rgba_t;

enum render_type_T {
    RENDER_TYPE_RECT,
    RENDER_TYPE_TEXTURE,
    RENDER_TYPE_CIRCLE,
    RENDER_TYPE_RECT_OUTLINE,
    RENDER_TYPE_CIRCLE_OUTLINE,
};

enum render_flags_t {
    RENDER_WIREFRAME = 0x1,
};

struct render_rect_t {
    // required:
    u32 type;
    v2 center;
    b32 parallax;

    v2 diagonal;
    f32 orientation;
    color_t color;
};

struct render_texture_t {
    // required:
    u32 type;
    v2 center;
    b32 parallax;


    rgba_t tint;
    tex2 texture;
    v2 hotspot;
    rect_i source_rect;
    f32 pixel_size;
    f32 orientation;
};

struct render_circle_t {
    // required:
    u32 type;
    v2 center;
    b32 parallax;

    
    f32 radius;
    color_t color;
};

struct render_object_t {
    f32 z;
    union {
        struct {
          u32 type;
          v2 center;
          b32 parallax;
        };
        render_rect_t render_rect;
        render_texture_t render_texture;
        render_circle_t render_circle;
    };
};

struct frame_buffer_t {
    u32 id;
    u32 texture;
    i32 width;
    i32 height;
};

struct render_group_t {
    frame_buffer_t frame_buffer;
    rgba_t lighting;
    vec<render_object_t> objects;
};

struct render_task_t {
    window_description_t buffer;
    camera_t camera;
    render_group_t* render_group;
    rect_i clip_rect;
};


enum gl_resouce_t {
    RES_SOLIDS_PROG,
    RES_SOLIDS_VAO_RECT,
    RES_SOLIDS_VAO_CIRCLE,
    RES_SOLIDS_VERTEX_MODELSPACE,
    RES_SOLIDS_DRAW_COLOR,
    RES_SOLIDS_TRANSFORM,
    RES_SOLIDS_LIGHTING,

    RES_TEXTURES_PROG,
    RES_TEXTURES_VAO_RECT,
    RES_TEXTURES_VERTEX_MODELSPACE,
    RES_TEXTURES_VERTEX_UV,
    RES_TEXTURES_TINT,
    RES_TEXTURES_TRANSFORM,
    RES_TEXTURES_UV_TRANSFORM,
    RES_TEXTURES_SAMPLER,
    RES_TEXTURES_LIGHTING,

    RES_SOLID_PARTICLES_PROG,
    RES_SOLID_PARTICLES_VAO_RECT,
    RES_SOLID_PARTICLES_VERTEX_MODELSPACE,
    RES_SOLID_PARTICLES_CENTER,
    RES_SOLID_PARTICLES_SCALING,
    RES_SOLID_PARTICLES_DRAW_COLOR,
    RES_SOLID_PARTICLES_CENTER_BUFFER,
    RES_SOLID_PARTICLES_SCALING_BUFFER,
    RES_SOLID_PARTICLES_DRAW_COLOR_BUFFER,
    RES_SOLID_PARTICLES_VIEW_TRANSFORM,
    RES_SOLID_PARTICLES_LIGHTING,

    RES_GRADIENT_PROG,
    RES_GRADIENT_VAO_RECT,
    RES_GRADIENT_VERTEX_MODELSPACE,
    RES_GRADIENT_VIEWPORT,
    RES_GRADIENT_START_COLOR,
    RES_GRADIENT_END_COLOR,
    RES_GRADIENT_GRADIENT_START,
    RES_GRADIENT_GRADIENT_END,

    RES_COUNT,
};

enum gl_program_id_t {
    GL_PROG_SOLIDS,
    GL_PROG_TEXTURES,
    GL_PROG_SOLID_PARTICLES,

    GL_PROG_COUNT,
};

enum gl_vao_id_t {
    GL_VAO_RECT,
    GL_VAO_CIRCLE,
    GL_VAO_COUNT,
};

enum gl_attrib_id_t {
    GL_ATTRIB_VERTEX_MODELSPACE,
    GL_ATTRIB_VERTEX_UV,

    GL_ATTRIB_PARTICLE_CENTER,
    GL_ATTRIB_PARTICLE_SCALING,
    GL_ATTRIB_PARTICLE_DRAW_COLOR,

    GL_ATTRIB_COUNT,
};

enum gl_uniform_id_t {
    GL_UNIFORM_MAIN_TRANSFORM,
    GL_UNIFORM_UV_TRANSFORM,

    GL_UNIFORM_SAMPLER,

    GL_UNIFORM_DRAW_COLOR,

    GL_UNIFORM_LIGHTING,

    GL_UNIFORM_COUNT,
};

enum gl_dynamic_vbo_id_t {
    GL_DYNAMIC_CENTERS,
    GL_DYNAMIC_SCALING,
    GL_DYNAMIC_DRAW_COLOR,

    GL_DYNAMIC_BUFFER_COUNT,
};

struct gl_program_t {
    u32 id;
    u32 vaos[GL_VAO_COUNT];
    u32 dynamic_buffers[GL_DYNAMIC_BUFFER_COUNT];
    i32 attribs[GL_ATTRIB_COUNT];
    i32 uniforms[GL_UNIFORM_COUNT];
};

struct gl_programs_t {
    union {
        i32 i_res_ids[RES_COUNT];
        u32 u_res_ids[RES_COUNT];
    };
    gl_program_t programs[GL_PROG_COUNT];
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


gl_programs_t load_programs();

frame_buffer_t default_frame_buffer(i32 width, i32 height);
frame_buffer_t create_frame_buffer(i32 width, i32 height);

tex2 load_image(char* filename);
tex2 load_bmp(char* filename, i32 scaling);

render_object_t* push_rect(render_group_t* render_group,
                           color_t color,
                           v2 center,
                           v2 diagonal,
                           f32 orientation,
                           f32 z,
                           b32 parallax = false);

render_object_t* push_rect_outline(render_group_t* render_group,
                                   color_t color,
                                   v2 center,
                                   v2 diagonal,
                                   f32 orientation,
                                   f32 z,
                                   b32 parallax = false);

render_object_t* push_circle(render_group_t* render_group,
                             color_t color,
                             v2 center,
                             f32 radius,
                             f32 z,
                             b32 parallax = false);

render_object_t* push_texture(render_group_t* render_group,
                              v2 center,
                              v2 hotspot,
                              f32 pixel_size,
                              tex2 texture,
                              rect_i source_rect,
                              rgba_t tint,
                              f32 orientation,
                              f32 z,
                              b32 parallax = false);

struct transient_state_t;

void draw_render_group(transient_state_t* transient_state,
                       gl_programs_t* programs,
                       camera_t camera,
                       render_group_t* render_group);

void clear_render_group(render_group_t render_group);

void draw_bmp(window_description_t buffer,
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

void draw_bmp(window_description_t buffer,
              tex2 bitmap,
              i32 dest_hotspot_x, i32 dest_hotspot_y,
              f32 source_pixel_size,
              u32 source_left, u32 source_top,
              u32 source_width, u32 source_height,
              f32 source_hotspot_x, f32 source_hotspot_y, f32 orientation);

void draw_circ_outline(window_description_t buffer,
                       rect_i clip_rect,
                       color_t color,
                       v2 center,
                       f32 radius);

void draw_rectangle(window_description_t buffer,
                    rect_i clip_rect,
                    color_t color, i32 center_x, i32 center_y, u32 width, u32 height,
                    f32 orientation = 0.0f);


void draw_rectangle(window_description_t buffer,
                    color_t color, i32 center_x, i32 center_y, u32 width, u32 height,
                    f32 orientation = 0.0f);

void add_debug_point(v2 point, i32 color);
void draw_debug_points();
void present_seconary_buffer();

#endif //PHYSICA_GAME_RENDER_H

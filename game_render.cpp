//
// Created by doug on 4/16/15.
//

#include <assert.h>
#include "physica_math.h"
#include "game_render.h"
#include "intrinsics.h"
#include "game.h"

tex2 load_bmp(char* filename) {
    tex2 result = {};
    platform_read_entire_file_result_t file =
        platform_read_entire_file(filename);

    if (file.contents) {
        bitmap_header_t* header = (bitmap_header_t*)file.contents;

        assert(header->compression == 3);

        assert(header->width > 0);
        assert(header->height > 0);
        result.width = (u32) header->width;
        result.height = (u32) header->height;
        result.pixels = (u32*)(file.contents + header->bitmap_offset);

        u32 red_mask = header->red_mask;
        u32 blue_mask = header->blue_mask;
        u32 green_mask = header->green_mask;
        u32 alpha_mask = ~(red_mask | green_mask | blue_mask);

        u32 red_shift = find_least_significant_set_bit(red_mask);
        u32 green_shift = find_least_significant_set_bit(green_mask);
        u32 blue_shift = find_least_significant_set_bit(blue_mask);
        u32 alpha_shift = find_least_significant_set_bit(alpha_mask);

        assert(red_shift >= 0);
        assert(green_shift >= 0);
        assert(blue_shift >= 0);
        assert(alpha_shift >= 0) ;

        u32* pixels = result.pixels;

        for (int i = 0; i < header->height; ++i) {
            for (int j = 0; j < header->width; ++j) {
                u32 color = pixels[i*header->width + j];
                pixels[i*header->width + j] =
                        (((color & alpha_mask) >> alpha_shift) << 24) |
                        (((color & red_mask) >> red_shift) << 16) |
                        (((color & green_mask) >> green_shift) << 8) |
                        (((color & blue_mask) >> blue_shift) << 0);
            }
        }
    }

    return result;
}

void draw_bmp(tex2 dest,
              tex2 source,
              i32 dest_hotspot_x, i32 dest_hotspot_y,
              f32 source_pixel_size,
              u32 source_left, u32 source_top,
              u32 source_width, u32 source_height,
              f32 source_hotspot_x, f32 source_hotspot_y, f32 orientation) {

    assert(source.width >= (source_left + source_width));
    assert(source.height >= (source_top + source_height));

    f32 min_x = 0;
    f32 min_y = 0;
    f32 max_x = source_width;
    f32 max_y = source_height;

    f32 sin_val = sin(orientation);
    f32 cos_val = cos(orientation);

    m3x3 rotation;
    rotation.r1 = {cos_val, sin_val, 0};
    rotation.r2 = {-sin_val, cos_val, 0};
    rotation.r3 = {0, 0, 1};

    m3x3 inverse_rotate;
    inverse_rotate.r1 = {cos_val, -sin_val, 0};
    inverse_rotate.r2 = {sin_val, cos_val, 0};
    inverse_rotate.r3 = {0, 0, 1};

    m3x3 scale;
    scale.r1 = {(f32)source_pixel_size, 0, 0};
    scale.r2 = {0, (f32)source_pixel_size, 0};
    scale.r3 = {0, 0, 1};

    m3x3 inverse_scale;
    inverse_scale.r1 = {1.0f / (f32)source_pixel_size, 0, 0};
    inverse_scale.r2 = {0, 1.0f / (f32)source_pixel_size, 0};
    inverse_scale.r3 = {0, 0, 1};

    m3x3 dest_translate;
    dest_translate.r1 = {1, 0, -(f32)dest_hotspot_x};
    dest_translate.r2 = {0, 1, -(f32)dest_hotspot_y};
    dest_translate.r3 = {0, 0, 1};

    m3x3 source_translate;
    source_translate.r1 = {1, 0, source_hotspot_x};
    source_translate.r2 = {0, 1, source_hotspot_y};
    source_translate.r3 = {0, 0, 1};

    v3 to_top_left = scale *
        (v3_from_ints(-(i32)source_hotspot_x,
            -(i32)source_hotspot_y));
    v3 to_bottom_left = scale *
        (v3_from_ints(-(i32)source_hotspot_x,
            source_height - (i32)source_hotspot_y));
    v3 to_top_right = scale *
        (v3_from_ints(source_width -
            (i32)source_hotspot_x,
            -(i32)source_hotspot_y));
    v3 to_bottom_right = scale *
        (v3_from_ints(source_width -
            (i32)source_hotspot_x, source_height -
            (i32)source_hotspot_y));

    i32 start_x = dest_hotspot_x;
    i32 end_x = dest_hotspot_x;
    i32 start_y = dest_hotspot_y;
    i32 end_y = dest_hotspot_y;

    if (sin_val > 0) {
        if (cos_val > 0) {
            start_x += (i32)((rotation * to_top_left).x);
            start_y += (i32)((rotation * to_top_right).y);
            end_x += (i32)((rotation * to_bottom_right).x);
            end_y += (i32)((rotation * to_bottom_left).y);
        } else {
            start_x += (i32)((rotation * to_top_right).x);
            start_y += (i32)((rotation * to_bottom_right).y);
            end_x += (i32)((rotation * to_bottom_left).x);
            end_y += (i32)((rotation * to_top_left).y);
        }
    } else{
        if (cos_val < 0) {
            start_x += (i32)((rotation * to_bottom_right).x);
            start_y += (i32)((rotation * to_bottom_left).y);
            end_x += (i32)((rotation * to_top_left).x);
            end_y += (i32)((rotation * to_top_right).y);
        } else {
            start_x += (i32)((rotation * to_bottom_left).x);
            start_y += (i32)((rotation * to_top_left).y);
            end_x += (i32)((rotation * to_top_right).x);
            end_y += (i32)((rotation * to_bottom_right).y);
        }
    }

    start_x = (i32) fmax(0, start_x);
    start_y = (i32) fmax(0, start_y);
    end_x = (i32) fmin(dest.width, end_x);
    end_y = (i32) fmin(dest.height, end_y);
    u32* pixels = dest.pixels;

    m3x3 transform =
            source_translate *
            inverse_rotate *
            inverse_scale *
            dest_translate;

    for (int y = start_y; y < end_y; ++y) {
        for (int x = start_x; x < end_x; ++x) {
            v3 p_d = v3_from_ints(x, y);
            v3 p_s = transform * p_d;
            if (p_s.x >= min_x && p_s.y >= min_y &&
                p_s.x < max_x && p_s.y < max_y) {
                u32 source_y = source.height - (i32)p_s.y - source_top - 2;
                u32 source_x = source.width + (i32)p_s.x + source_left;
                u32 val = source.pixels[source_y * source.width + source_x];
                if (val & 0xff000000) {
                    pixels[(dest.height - y - 1) * dest.width + x] = val;
                }
            }
        }
    }
}

void draw_bmp(video_buffer_description_t buffer,
              tex2 bitmap,
              i32 dest_hotspot_x, i32 dest_hotspot_y,
              f32 source_pixel_size,
              u32 source_left, u32 source_top,
              u32 source_width, u32 source_height,
              f32 source_hotspot_x, f32 source_hotspot_y, f32 orientation) {

    assert(bitmap.width >= (source_left + source_width));
    assert(bitmap.height >= (source_top + source_height));

    f32 min_x = 0;
    f32 min_y = 0;
    f32 max_x = source_width;
    f32 max_y = source_height;

    f32 sin_val = sin(orientation);
    f32 cos_val = cos(orientation);

    m3x3 rotation;
    rotation.r1 = {cos_val, sin_val, 0};
    rotation.r2 = {-sin_val, cos_val, 0};
    rotation.r3 = {0, 0, 1};

    m3x3 inverse_rotate;
    inverse_rotate.r1 = {cos_val, -sin_val, 0};
    inverse_rotate.r2 = {sin_val, cos_val, 0};
    inverse_rotate.r3 = {0, 0, 1};

    m3x3 scale;
    scale.r1 = {(f32)source_pixel_size, 0, 0};
    scale.r2 = {0, (f32)source_pixel_size, 0};
    scale.r3 = {0, 0, 1};

    m3x3 inverse_scale;
    inverse_scale.r1 = {1.0f / (f32)source_pixel_size, 0, 0};
    inverse_scale.r2 = {0, 1.0f / (f32)source_pixel_size, 0};
    inverse_scale.r3 = {0, 0, 1};

    m3x3 dest_translate;
    dest_translate.r1 = {1, 0, -(f32)dest_hotspot_x};
    dest_translate.r2 = {0, 1, -(f32)dest_hotspot_y};
    dest_translate.r3 = {0, 0, 1};

    m3x3 source_translate;
    source_translate.r1 = {1, 0, source_hotspot_x};
    source_translate.r2 = {0, 1, source_hotspot_y};
    source_translate.r3 = {0, 0, 1};

    v3 to_top_left = scale *
            (v3_from_ints(-(i32)source_hotspot_x, -(i32)source_hotspot_y));
    v3 to_bottom_left = scale *
            (v3_from_ints(-(i32)source_hotspot_x,
            source_height - (i32)source_hotspot_y));
    v3 to_top_right = scale *
            (v3_from_ints(source_width - (i32)source_hotspot_x,
            -(i32)source_hotspot_y));
    v3 to_bottom_right = scale *
            (v3_from_ints(source_width -
            (i32)source_hotspot_x, source_height - (i32)source_hotspot_y));

    i32 start_x = dest_hotspot_x;
    i32 end_x = dest_hotspot_x;
    i32 start_y = dest_hotspot_y;
    i32 end_y = dest_hotspot_y;

    if (sin_val > 0) {
        if (cos_val > 0) {
            start_x += (i32)((rotation * to_top_left).x);
            start_y += (i32)((rotation * to_top_right).y);
            end_x += (i32)((rotation * to_bottom_right).x);
            end_y += (i32)((rotation * to_bottom_left).y);
        } else {
            start_x += (i32)((rotation * to_top_right).x);
            start_y += (i32)((rotation * to_bottom_right).y);
            end_x += (i32)((rotation * to_bottom_left).x);
            end_y += (i32)((rotation * to_top_left).y);
        }
    } else{
        if (cos_val < 0) {
            start_x += (i32)((rotation * to_bottom_right).x);
            start_y += (i32)((rotation * to_bottom_left).y);
            end_x += (i32)((rotation * to_top_left).x);
            end_y += (i32)((rotation * to_top_right).y);
        } else {
            start_x += (i32)((rotation * to_bottom_left).x);
            start_y += (i32)((rotation * to_top_left).y);
            end_x += (i32)((rotation * to_top_right).x);
            end_y += (i32)((rotation * to_bottom_right).y);
        }
    }

    start_x = (i32) fmax(0, start_x);
    start_y = (i32) fmax(0, start_y);
    end_x = (i32) fmin(buffer.width, end_x);
    end_y = (i32) fmin(buffer.height, end_y);
    u32* pixels = (u32*)buffer.memory;

    m3x3 transform = source_translate * inverse_rotate * inverse_scale * dest_translate;

    for (int y = start_y; y < end_y; ++y) {
        for (int x = start_x; x < end_x; ++x) {
            v3 p_d = v3_from_ints(x, y);
            v3 p_s = transform * p_d;
            if (p_s.x >= min_x && p_s.y >= min_y &&
                p_s.x < max_x && p_s.y < max_y) {
                u32 source_y = bitmap.height - (i32)p_s.y - source_top - 2;
                u32 source_x = bitmap.width + (i32)p_s.x + source_left;
                u32 val = bitmap.pixels[source_y * bitmap.width + source_x];
                if (val & 0xff000000) {
                    pixels[y * buffer.width + x] = val;
                }
            }
        }
    }
}

static i32 debug_point_count = 0;
static v2 debug_points[200];
static i32 debug_colors[200];

void add_debug_point(v2 point, i32 color) {
    debug_points[debug_point_count] = point;
    debug_colors[debug_point_count++] = color;
}

static i32 debug_rect_count = 0;
static v2 debug_rects[20000];
static i32 debug_rect_colors[10000];

void add_debug_rect(v2 top_right, v2 bottom_left, i32 color) {
    debug_rects[debug_rect_count] = top_right;
    debug_rects[debug_rect_count+1] = bottom_left;
    debug_colors[debug_rect_count / 2] = color;
    debug_rect_count += 2;
}

void draw_debug_points(video_buffer_description_t buffer, camera_t camera) {
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

    for (int i = 0; i < debug_point_count; ++i) {
        v2 p = camera_space_transform * debug_points[i];
        draw_rectangle(buffer, debug_colors[i], p.x, p.y, 2, 2, 0.0f);
    }

    for (int i = 0; i < (debug_rect_count / 2); ++i) {
        v2 top_right = debug_rects[i*2];
        v2 bottom_left = debug_rects[i*2+1];
        v2 p = camera_space_transform * ((top_right + bottom_left) * 0.5f);
        v2 size = scale * (top_right - bottom_left);

        draw_rectangle(buffer, debug_rect_colors[i], p.x, p.y, size.x, size.y, 0.0f);
    }

    debug_point_count = 0;
    debug_rect_count = 0;
}

// NOTE(doug): orientation is counter-clockwise radians from the x axis
void draw_rectangle(video_buffer_description_t buffer,
                    u32 value, i32 center_x, i32 center_y, u32 width, u32 height,
                    f32 orientation) {

    f32 half_width = ((f32)width) / 2.0f;
    f32 half_height = ((f32)height) / 2.0f;

    v2 to_top_left = v2 {-half_width, -half_height};
    v2 to_bottom_left = v2 {-half_width, half_height};
    v2 to_top_right = -to_bottom_left;
    v2 to_bottom_right = -to_top_left;

    f32 min_x = center_x - half_width;
    f32 min_y = center_y - half_height;
    f32 max_x = center_x + half_width;
    f32 max_y = center_y + half_height;

    f32 sin_val = sin(orientation);
    f32 cos_val = cos(orientation);

    m2x2 rotation;
    rotation.r1 = {cos_val, sin_val};
    rotation.r2 = {-sin_val, cos_val};

    m3x3 inverse_rotate;
    inverse_rotate.r1 = {cos_val,-sin_val, 0};
    inverse_rotate.r2 = {sin_val, cos_val, 0};
    inverse_rotate.r3 = {0,0,1};

    i32 start_x = center_x;
    i32 end_x = center_x;
    i32 start_y = center_y;
    i32 end_y = center_y;

    if (sin_val > 0) {
        if (cos_val > 0) {
            start_x += (i32)((rotation * to_top_left).x);
            start_y += (i32)((rotation * to_top_right).y);
            end_x += (i32)((rotation * to_bottom_right).x);
            end_y += (i32)((rotation * to_bottom_left).y);
        } else {
            start_x += (i32)((rotation * to_top_right).x);
            start_y += (i32)((rotation * to_bottom_right).y);
            end_x += (i32)((rotation * to_bottom_left).x);
            end_y += (i32)((rotation * to_top_left).y);
        }
    } else {
        if (cos_val < 0) {
            start_x += (i32)((rotation * to_bottom_right).x);
            start_y += (i32)((rotation * to_bottom_left).y);
            end_x += (i32)((rotation * to_top_left).x);
            end_y += (i32)((rotation * to_top_right).y);
        } else {
            start_x += (i32)((rotation * to_bottom_left).x);
            start_y += (i32)((rotation * to_top_left).y);
            end_x += (i32)((rotation * to_top_right).x);
            end_y += (i32)((rotation * to_bottom_right).y);
        }
    }

    start_x = (i32) fmax(0, start_x - 1);
    start_y = (i32) fmax(0, start_y - 1);
    end_x = (i32) fmin(buffer.width, end_x + 1);
    end_y = (i32) fmin(buffer.height, end_y + 1);

    v2 center = v2 {(f32)center_x, (f32)center_y};
    u32* pixels = (u32*)buffer.memory;

    m3x3 transform = get_translation_matrix(center) *
            inverse_rotate *
            get_translation_matrix(-center);

    v3 rgb = to_rgb(value);
    for (int y = start_y; y < end_y; ++y) {
        for (int x = start_x; x < end_x; ++x) {
            v2 p_original = transform * v2 { (f32)x, (f32)y };
            f32 diff_x = fmax(min_x - p_original.x, p_original.x - max_x);
            f32 diff_y = fmax(min_y - p_original.y, p_original.y - max_y);

            diff_x = fclamp(diff_x + 0.5f, 0.0f, 1.0f);
            diff_y = fclamp(diff_y + 0.5f, 0.0f, 1.0f);
            f32 ratio = (1.0f - diff_x) * (1.0f - diff_y);
            v3 existing = to_rgb(pixels[y * buffer.width + x]);
            pixels[y * buffer.width + x] =
                    from_rgb((1.0f - ratio) * existing + ratio * rgb);
        }
    }
}
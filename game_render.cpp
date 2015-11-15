//
// Created by doug on 4/16/15.
//

#include <assert.h>
#include "physica_math.h"
#include "game_render.h"
#include "intrinsics.h"
#include "game.h"
#include "gl/gl.h"
#include "camera.h"

tex2 load_bmp(char* filename, i32 scaling) {
    tex2 result = {};
    platform_read_entire_file_result_t file = platform_read_entire_file(filename);

    assert_(file.contents);

    if (file.contents) {
        bitmap_header_t* header = (bitmap_header_t*)file.contents;

        assert_(header->compression == 3);
        assert_(header->width > 0);
        assert_(header->height > 0);

        result.width = (u32) header->width * scaling;
        result.pitch = result.width;
        result.height = (u32) header->height * scaling;
        result.pixels = (u32*)malloc(result.width * result.height * sizeof(u32));

        u32 red_mask = header->red_mask;
        u32 blue_mask = header->blue_mask;
        u32 green_mask = header->green_mask;
        u32 alpha_mask = ~(red_mask | green_mask | blue_mask);

        u32 red_shift = find_least_significant_set_bit(red_mask);
        u32 green_shift = find_least_significant_set_bit(green_mask);
        u32 blue_shift = find_least_significant_set_bit(blue_mask);
        u32 alpha_shift = find_least_significant_set_bit(alpha_mask);

        assert_(red_shift >= 0);
        assert_(green_shift >= 0);
        assert_(blue_shift >= 0);
        assert_(alpha_shift >= 0) ;

        u32* input = (u32*)(file.contents + header->bitmap_offset);
        u32* output = result.pixels;
        i32 width = header->width * scaling;

        for (int i = 0; i < header->height; ++i) {
            for (int j = 0; j < header->width; ++j) {
                u32 color = input[i*header->width + j];
                color = (((color & alpha_mask) >> alpha_shift) << 0) |
                        (((color & red_mask) >> red_shift) << 24) |
                        (((color & green_mask) >> green_shift) << 16) |
                        (((color & blue_mask) >> blue_shift) << 8);

                for (int k = 0; k < scaling; ++k) {
                    for (int l = 0; l < scaling; ++l) {
                        output[(i * scaling + k)*width + (j * scaling) + l] = color;
                    }
                }
            }
        }

        glGenTextures(1, &result.texture_id);
        glBindTexture(GL_TEXTURE_2D, result.texture_id);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA,
                     result.width,
                     result.height,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_INT_8_8_8_8,
                     (void*)output);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    return result;
}

render_object_t* push_circle(render_group_t* render_group,
                             color_t color,
                             v2 center,
                             f32 radius,
                             f32 z,
                             u32 flags) {
    i32 i = render_group->objects.push_unassigned();
    render_object_t* obj = render_group->objects.at(i);
    obj->type = RENDER_TYPE_CIRCLE;
    obj->render_circle.color = color;
    obj->render_circle.center = center;
    obj->render_circle.radius = radius;
    obj->z = z;
    obj->flags = flags;
    return obj;
}

render_object_t* push_rect(render_group_t* render_group,
                           color_t color,
                           v2 center,
                           v2 diagonal,
                           f32 orientation,
                           f32 z,
                           u32 flags) {
    i32 i = render_group->objects.push_unassigned();
    render_object_t* obj = render_group->objects.at(i);
    obj->type = RENDER_TYPE_RECT;
    obj->render_rect.color = color;
    obj->render_rect.center = center;
    obj->render_rect.diagonal = diagonal;
    obj->render_rect.orientation = orientation;
    obj->z = z;
    obj->flags = flags;
    return obj;
}

render_object_t* push_texture(render_group_t* render_group,
                              v2 center,
                              v2 hotspot,
                              f32 pixel_size,
                              tex2 texture,
                              rect_i source_rect,
                              f32 orientation,
                              f32 z) {
    i32 i = render_group->objects.push_unassigned();
    render_object_t* obj = render_group->objects.at(i);
    obj->type = RENDER_TYPE_TEXTURE;
    obj->flags = 0;
    obj->render_texture.texture = texture;
    obj->render_texture.source_rect = source_rect;
    obj->render_texture.pixel_size = pixel_size;
    obj->render_texture.center = center;
    obj->render_texture.hotspot = hotspot;
    obj->render_texture.orientation = orientation;
    obj->z = z;
    return obj;
}

void swap_render_objs(render_object_t* a, render_object_t* b) {
    render_object_t tmp = *a;
    *a = *b;
    *b = tmp;
}

void sort_render_objects(vec<render_object_t> objs) {
    // sort our render objects by z
    const i32 quicksort_max_stack_size = 1024;
    i32 si = 0;
    tuple2<i32,i32> stack[quicksort_max_stack_size];
    stack[si++] = {0, objs.count};

    while (si) {
        tuple2<i32,i32> current = stack[--si];

        if (current.second - current.first <= 1) {
            continue;
        }

        i32 pivot = current.first;
        i32 left = current.first;
        i32 right = current.second - 1;
        render_object_t pivot_val = objs[pivot];
        f32 pivot_z = pivot_val.z;

        while (left < right) {
            while (left < current.second && (objs[left]).z >= pivot_z) { ++left; }
            while ((objs[right]).z < pivot_z) { --right; }
            if (left < right && left < current.second) {
                swap_render_objs(objs.at(left), objs.at(right));
            }
        }

        objs[current.first] = objs[right];
        objs[right] = pivot_val;

        pivot = right;

        stack[si++] = {current.first, pivot};
        stack[si++] = {pivot + 1, current.second};
    }
}

void draw_gl_rect(platform_services_t platform,
                  camera_t camera,
                  render_rect_t rect,
                  f32 z) {
    // our base rect is simply a square at the origin with sides of length 1.0f,
    // transform that square to get our draw rect

    m4x4 model_rotate = get_rotation_matrix_4x4(rect.orientation);
    m4x4 model_scale = identity_4x4();
    m4x4 model_translate = identity_4x4();
    //scale
    model_scale.r1.c1 = rect.diagonal.x;
    model_scale.r2.c2 = rect.diagonal.y;
    // translate
    model_translate.r1.c4 = rect.center.x;
    model_translate.r2.c4 = rect.center.y;
    model_translate.r3.c4 = z;

    m4x4 model = model_translate * model_rotate * model_scale;
    m4x4 view = get_view_transform_4x4(camera);
    m4x4 transform = view * model;

    glUniformMatrix4fv(platform.basic_program.transform_loc,
                       1,
                       GL_TRUE,
                       transform.vals);
    glUniform4f(platform.basic_program.color_loc,
                rect.color.r,
                rect.color.g,
                rect.color.b,
                1.0f);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);
}

void draw_gl_circle(platform_services_t platform,
                    camera_t camera,
                    render_circle_t circle,
                    f32 z,
                    u32 flags) {
    // our base rect is simply a square at the origin with sides of length 1.0f,
    // transform that square to get our draw rect

    m4x4 model_rotate = identity_4x4();
    m4x4 model_scale = identity_4x4();
    m4x4 model_translate = identity_4x4();
    //scale
    model_scale.r1.c1 = circle.radius;
    model_scale.r2.c2 = circle.radius;
    // translate
    model_translate.r1.c4 = circle.center.x;
    model_translate.r2.c4 = circle.center.y;
    model_translate.r3.c4 = z;

    m4x4 model = model_translate * model_rotate * model_scale;
    m4x4 view = get_view_transform_4x4(camera);
    
    m4x4 transform = view * model;

    glUniformMatrix4fv(platform.basic_program.transform_loc,
                       1,
                       GL_TRUE,
                       transform.vals);
    glUniform4f(platform.basic_program.color_loc,
                circle.color.r,
                circle.color.g,
                circle.color.b,
                1.0f);
    if (flags & RENDER_WIREFRAME) {
        glDrawArrays(GL_LINE_STRIP, 1, platform.basic_program.ellipse_ibo_count - 1);
    } else {
        glDrawArrays(GL_TRIANGLE_FAN, 0, platform.basic_program.ellipse_ibo_count);
    }
}

void draw_gl_texture(platform_services_t platform,
                     camera_t camera,
                     render_texture_t texture,
                     f32 z) {
    f32 src_width = (f32)(texture.source_rect.max_x - texture.source_rect.min_x); 
    f32 src_height = (f32)(texture.source_rect.max_y - texture.source_rect.min_y);

    f32 width = src_width * texture.pixel_size;
    f32 height = src_height * texture.pixel_size;


    // our base rect is simply a square at the origin with sides of length 1.0f,
    // transform that square to get our draw rect

    v2 hotspot_transform = -1.0f * (v2 {-width / 2.0f, -height / 2.0f} +
                                    (texture.hotspot * texture.pixel_size));
    m4x4 hotspot_translate = identity_4x4();
    m4x4 model_rotate = get_rotation_matrix_4x4(texture.orientation);
    m4x4 model_scale = identity_4x4();
    m4x4 model_translate = identity_4x4();
    //scale
    model_scale.r1.c1 = width;
    model_scale.r2.c2 = height;
    // translate
    hotspot_translate.r1.c4 = hotspot_transform.x;
    hotspot_translate.r2.c4 = hotspot_transform.y;
    model_translate.r1.c4 = texture.center.x;
    model_translate.r2.c4 = texture.center.y;
    model_translate.r3.c4 = z;

    m4x4 model = model_translate * model_rotate * hotspot_translate * model_scale;

    f32 camera_scale_x = 1.0f / camera.to_top_left.x;
    f32 camera_scale_y = 1.0f / camera.to_top_left.y;
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

    m4x4 transform = view * model;

    m3x3 uv_transform = identity_3x3();
    uv_transform.r1.c1 = src_width / (f32)texture.texture.width;
    uv_transform.r2.c2 = src_height / (f32)texture.texture.height;
    uv_transform.r1.c3 = texture.source_rect.min_x / (f32)texture.texture.width;
    uv_transform.r2.c3 = texture.source_rect.min_y / (f32)texture.texture.height;

    glBindTexture(GL_TEXTURE_2D, texture.texture.texture_id);
    glUniform1i(platform.texture_program.texture_sampler_loc, 0);

    glUniformMatrix4fv(platform.texture_program.transform_loc,
                       1,
                       GL_TRUE,
                       transform.vals);
    glUniformMatrix3fv(platform.texture_program.uv_transform_loc,
                       1,
                       GL_TRUE,
                       uv_transform.vals);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);
}

void setup_gl_for_type(platform_services_t platform,
                       u32 type,
                       u32 flags) {

    if (flags & RENDER_WIREFRAME) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    switch (type) {
        case RENDER_TYPE_RECT: {
            glUseProgram(platform.basic_program.id);
            glBindVertexArray(platform.basic_program.vao);
        } break;
        case RENDER_TYPE_CIRCLE: {
            glUseProgram(platform.basic_program.id);
            glBindVertexArray(platform.basic_program.ellipse_outline_vao);
        } break;
        case RENDER_TYPE_TEXTURE: {
            glUseProgram(platform.texture_program.id);
            glBindVertexArray(platform.texture_program.vao);
            glActiveTexture(GL_TEXTURE0);
        } break;
    }
}

void present_frame_buffer(platform_services_t platform,
                          u32 color_buffer) {
    setup_gl_for_type(platform, RENDER_TYPE_TEXTURE, 0);

    glBindTexture(GL_TEXTURE_2D, color_buffer);
    glUniform1i(platform.texture_program.texture_sampler_loc, 0);

    m4x4 transform = identity_4x4();
    transform.r1.c1 = 2.0f;
    transform.r2.c2 = 2.0f;
    m3x3 uv_transform = identity_3x3();
    glUniformMatrix4fv(platform.texture_program.transform_loc,
                       1,
                       GL_TRUE,
                       transform.vals);
    glUniformMatrix3fv(platform.texture_program.uv_transform_loc,
                       1,
                       GL_TRUE,
                       uv_transform.vals);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);

}

void draw_render_group(platform_services_t platform,
                       video_buffer_description_t buffer,
                       camera_t camera,
                       render_group_t* render_group) {
    TIMED_FUNC();

#if 1
    sort_render_objects(render_group->objects);

    static u32 previous_type = -1;
    static u32 previous_flags = -1;
    for (int i = 0; i < render_group->objects.count; ++i) {
        render_object_t* obj = render_group->objects.at(i);

        if (obj->type != previous_type || obj->flags != previous_flags) {
            setup_gl_for_type(platform, obj->type, obj->flags);
        }

        switch (obj->type) {
            case RENDER_TYPE_RECT: {
                draw_gl_rect(platform,
                             camera,
                             obj->render_rect,
                             obj->z);
            } break;
            case RENDER_TYPE_CIRCLE: {
                draw_gl_circle(platform,
                               camera,
                               obj->render_circle,
                               obj->z,
                               obj->flags);
            } break;
            case RENDER_TYPE_TEXTURE: {
                draw_gl_texture(platform,
                                camera,
                                obj->render_texture,
                                obj->z);
            } break;
        }

        previous_type = obj->type;
        previous_flags = obj->flags;
    }

#else
    i32 clip_width = 256;
    i32 clip_height = 256;

    render_task_t tasks[256] = {0};

    i32 width_in_chunks = buffer.width / clip_width + 1;
    i32 height_in_chunks = buffer.height / clip_height + 1;

    sort_render_objects(render_group->objects);

    render_task_t* task = tasks;
    for (i32 i = 0; i < height_in_chunks; ++i) {
        for (i32 j = 0; j < width_in_chunks; ++j) {
            task->clip_rect = {
                j*clip_width, i*clip_height,
                (j+1)*clip_width, (i+1) * clip_height
            };
            task->clip_rect.max_x = fmin(buffer.width, task->clip_rect.max_x);
            task->clip_rect.max_y = fmin(buffer.height, task->clip_rect.max_y);
            task->buffer = buffer;
            task->camera = camera;
            task->render_group = render_group;

            platform.start_task(platform.render_queue, &run_render_task, task);
            task++;
        }
    }

    platform.wait_on_queue(platform.render_queue);
#endif
}

void clear_render_group(render_group_t* render_group) {
    render_group->objects.count = 0;
}

inline b32 is_in_rect(v2 p, rect_i clip) {
    return p.x >= clip.min_x && p.y >= clip.min_y && p.x < clip.max_x && p.y < clip.max_y;
}

inline b32 is_in_rect(i32 x, i32 y, rect_i clip) {
    return x >= clip.min_x && y >= clip.min_y && x < clip.max_x && y < clip.max_y;
}

void draw_bmp(video_buffer_description_t buffer,
              rect_i clip_rect,
              tex2 bitmap,
              v2 center,
              f32 source_pixel_size,
              v2 hotspot,
              f32 orientation) {
    TIMED_FUNC();

    f32 min_x = 0;
    f32 min_y = 0;
    f32 max_x = bitmap.width;
    f32 max_y = bitmap.height;

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
    scale.r1 = {source_pixel_size, 0, 0};
    scale.r2 = {0, source_pixel_size, 0};
    scale.r3 = {0, 0, 1};

    m3x3 inverse_scale;
    inverse_scale.r1 = {1.0f / source_pixel_size, 0, 0};
    inverse_scale.r2 = {0, 1.0f / source_pixel_size, 0};
    inverse_scale.r3 = {0, 0, 1};

    m3x3 dest_translate;
    dest_translate.r1 = {1, 0, -center.x};
    dest_translate.r2 = {0, -1, center.y};
    dest_translate.r3 = {0, 0, 1};

    m3x3 source_translate;
    source_translate.r1 = {1, 0, hotspot.x};
    source_translate.r2 = {0, 1, hotspot.y};
    source_translate.r3 = {0, 0, 1};

    v3 to_top_left = scale * (-1 * to_v3(hotspot));
    v3 to_bottom_left = scale * (-1 * to_v3(hotspot) + v3{0,(f32)bitmap.height,0});
    v3 to_top_right = scale * (-1 * to_v3(hotspot) + v3{(f32)bitmap.width,0,0});
    v3 to_bottom_right = scale * (-1 * to_v3(hotspot) + v3{(f32)bitmap.width,(f32)bitmap.height,0});

    f32 start_x_f = center.x;
    f32 end_x_f = center.x;
    f32 start_y_f = center.y;
    f32 end_y_f = center.y;

    if (sin_val > 0) {
        if (cos_val > 0) {
            start_x_f += (rotation * to_top_left).x;
            start_y_f += (rotation * to_top_right).y;
            end_x_f += (rotation * to_bottom_right).x;
            end_y_f += (rotation * to_bottom_left).y;
        } else {
            start_x_f += (rotation * to_top_right).x;
            start_y_f += (rotation * to_bottom_right).y;
            end_x_f += (rotation * to_bottom_left).x;
            end_y_f += (rotation * to_top_left).y;
        }
    } else {
        if (cos_val < 0) {
            start_x_f += (rotation * to_bottom_right).x;
            start_y_f += (rotation * to_bottom_left).y;
            end_x_f += (rotation * to_top_left).x;
            end_y_f += (rotation * to_top_right).y;
        } else {
            start_x_f += (rotation * to_bottom_left).x;
            start_y_f += (rotation * to_top_left).y;
            end_x_f += (rotation * to_top_right).x;
            end_y_f += (rotation * to_bottom_right).y;
        }
    }


    i32 start_x = fmax(clip_rect.min_x, sround(start_x_f));
    i32 start_y = fmax(clip_rect.min_y, sround(2.0f * center.y - end_y_f));
    i32 end_x = fmin(clip_rect.max_x, fmax(0, sround(end_x_f)));
    i32 end_y = fmin(clip_rect.max_y, sround(2.0f * center.y - start_y_f));

    u32* pixels = (u32*)buffer.memory;

    m3x3 scale_and_rotate = 
            inverse_rotate *
            inverse_scale;

    m3x3 transform =
            source_translate *
            inverse_rotate *
            inverse_scale *
            dest_translate;

    rect_i bmp_rect;
    bmp_rect.min_x = min_x;
    bmp_rect.max_x = max_x;
    bmp_rect.min_y = min_y;
    bmp_rect.max_y = max_y;

    rgba_t transparent = {0};

    const i32 supersampling_vector_count = 4;
    v2 supersampling_vectors[supersampling_vector_count];

    f32 offs = 1.0f / 3.0f;
    supersampling_vectors[0] = scale_and_rotate * v2 {-offs, -offs};
    supersampling_vectors[1] = scale_and_rotate * v2 {offs, -offs};
    supersampling_vectors[2] = scale_and_rotate * v2 {-offs, offs};
    supersampling_vectors[3] = scale_and_rotate * v2 {offs, offs};

    __m128 supersampling_xs[supersampling_vector_count];
    __m128 supersampling_ys[supersampling_vector_count];

    for (int i = 0; i < supersampling_vector_count; ++i) {
        supersampling_xs[i] = _mm_set1_ps(supersampling_vectors[i].x);
        supersampling_ys[i] = _mm_set1_ps(supersampling_vectors[i].y);
    }

    __m128 transform_wide[9];

    for (int i = 0; i < 9; ++i) {
        transform_wide[i] = _mm_set1_ps(transform.vals[i]);
    }

    __m128 min_x_wide = _mm_set1_ps(min_x);
    __m128 max_x_wide = _mm_set1_ps(max_x);

    __m128 min_y_wide = _mm_set1_ps(min_y);
    __m128 max_y_wide = _mm_set1_ps(max_y);

    __m128 wide_1 = _mm_set1_ps(1.0f);
    __m128 wide_half = _mm_set1_ps(0.5f);
    __m128 wide_0 = _mm_set1_ps(0.0f);
    __m128 wide_4 = _mm_set1_ps(4.0f);
    __m128 wide_255 = _mm_set1_ps(255.0f);
    __m128 wide_inv255 = _mm_set1_ps(1.0f / 255.0f);
    __m128 wide_inv_supersampling_count = _mm_set1_ps(1.0f / (f32)supersampling_vector_count);

    __m128i a_mask = _mm_set1_epi32(0xff000000);
    __m128i r_mask = _mm_set1_epi32(0x00ff0000);
    __m128i g_mask = _mm_set1_epi32(0x0000ff00);
    __m128i b_mask = _mm_set1_epi32(0x000000ff);

    for (int y = start_y; y < end_y; ++y) {
        __m128 ys = _mm_set1_ps((f32)y);
        __m128 xs = _mm_setr_ps((f32)start_x,
                                 (f32)start_x+1.0f,
                                 (f32)start_x+2.0f,
                                 (f32)start_x+3.0f);

        __m128 first_transformed_ys = 
            _mm_add_ps(_mm_mul_ps(transform_wide[1], ys),
                       transform_wide[2]);
        __m128 second_transformed_ys = 
            _mm_add_ps(_mm_mul_ps(transform_wide[4], ys),
                       transform_wide[5]);

        i32 x;
        for (x = start_x; x < end_x - 4; x += 4) {
            __m128 xs_trans =
                _mm_add_ps(_mm_mul_ps(transform_wide[0], xs), first_transformed_ys);
            __m128 ys_trans =
                _mm_add_ps(_mm_mul_ps(transform_wide[3], xs), second_transformed_ys);

            __m128 composite_a = wide_0;
            __m128 composite_r = wide_0;
            __m128 composite_g = wide_0;
            __m128 composite_b = wide_0;

            for (int i = 0; i < supersampling_vector_count; ++i) {
                __m128 pxs = _mm_add_ps(xs_trans, supersampling_xs[i]);
                __m128 pys = _mm_add_ps(ys_trans, supersampling_ys[i]);

                __m128i wide_xsi = _mm_cvttps_epi32(pxs);
				__m128i wide_ysi = _mm_cvttps_epi32(pys);
                i32* xsi = (i32*)&wide_xsi;
                i32* ysi = (i32*)&wide_ysi;

                __m128i wide_pixs;
                i32* pixs = (i32*)&wide_pixs;
                pixs[0] = is_in_rect(xsi[0], ysi[0], bmp_rect) ?
                    bitmap.pixels[ysi[0] * bitmap.pitch + xsi[0]] : 0;
                pixs[1] = is_in_rect(xsi[1], ysi[1], bmp_rect) ?
                    bitmap.pixels[ysi[1] * bitmap.pitch + xsi[1]] : 0;
                pixs[2] = is_in_rect(xsi[2], ysi[2], bmp_rect) ?
                    bitmap.pixels[ysi[2] * bitmap.pitch + xsi[2]] : 0;
                pixs[3] = is_in_rect(xsi[3], ysi[3], bmp_rect) ?
                    bitmap.pixels[ysi[3] * bitmap.pitch + xsi[3]] : 0;

                __m128 pix_as = _mm_mul_ps(wide_inv255,_mm_cvtepi32_ps(_mm_srli_epi32(wide_pixs, 24)));
                __m128 inv_a_255 = _mm_mul_ps(wide_inv255, pix_as);

                composite_a = _mm_add_ps(composite_a, pix_as);

                composite_r = _mm_add_ps(composite_r,
                                         _mm_mul_ps(inv_a_255,
                                         _mm_cvtepi32_ps(
                                         _mm_srli_epi32(_mm_and_si128(wide_pixs, r_mask), 16))));
                composite_g = _mm_add_ps(composite_g,
                                         _mm_mul_ps(inv_a_255,
                                         _mm_cvtepi32_ps(
                                         _mm_srli_epi32(_mm_and_si128(wide_pixs, g_mask), 8))));
                composite_b = _mm_add_ps(composite_b,
                                         _mm_mul_ps(inv_a_255,
                                         _mm_cvtepi32_ps(_mm_and_si128(wide_pixs, b_mask))));
            }

            __m128 inv_a = _mm_div_ps(wide_1, composite_a);
            __m128 neq = _mm_cmpneq_ps(composite_a, wide_0);

            composite_r = _mm_and_ps(_mm_mul_ps(inv_a, composite_r), neq);
            composite_g = _mm_and_ps(_mm_mul_ps(inv_a, composite_g), neq);
            composite_b = _mm_and_ps(_mm_mul_ps(inv_a, composite_b), neq);
            composite_a = _mm_mul_ps(composite_a, wide_inv_supersampling_count);

            __m128i* pixel_loc = (__m128i*)(pixels + y * buffer.width + x);

            __m128i existing_i = _mm_loadu_si128((__m128i*)pixel_loc);
            __m128i existing_a = _mm_srli_epi32(_mm_and_si128(existing_i, a_mask), 24);
            __m128i existing_r = _mm_srli_epi32(_mm_and_si128(existing_i, r_mask), 16);
            __m128i existing_g = _mm_srli_epi32(_mm_and_si128(existing_i, g_mask), 8);
            __m128i existing_b = _mm_and_si128(existing_i, b_mask);

            __m128 fexisting_a = _mm_mul_ps(wide_inv255, _mm_cvtepi32_ps(existing_a));
            __m128 fexisting_r = _mm_mul_ps(wide_inv255, _mm_cvtepi32_ps(existing_r));
            __m128 fexisting_g = _mm_mul_ps(wide_inv255, _mm_cvtepi32_ps(existing_g));
            __m128 fexisting_b = _mm_mul_ps(wide_inv255, _mm_cvtepi32_ps(existing_b));

            __m128 overlay_a =
                _mm_sub_ps(wide_1,
                           _mm_mul_ps(_mm_sub_ps(wide_1, fexisting_a),
                                      _mm_sub_ps(wide_1, composite_a)));
            __m128 overlay_r =
                _mm_add_ps(fexisting_r,
                           _mm_mul_ps(_mm_sub_ps(composite_r, fexisting_r), composite_a));
            __m128 overlay_g =
                _mm_add_ps(fexisting_g,
                           _mm_mul_ps(_mm_sub_ps(composite_g, fexisting_g), composite_a));
            __m128 overlay_b =
                _mm_add_ps(fexisting_b,
                           _mm_mul_ps(_mm_sub_ps(composite_b, fexisting_b), composite_a));

            __m128i pix_a = _mm_slli_epi32(_mm_cvtps_epi32(_mm_mul_ps(wide_255, overlay_a)), 24);
            __m128i pix_r = _mm_slli_epi32(_mm_cvtps_epi32(_mm_mul_ps(wide_255, overlay_r)), 16);
            __m128i pix_g = _mm_slli_epi32(_mm_cvtps_epi32(_mm_mul_ps(wide_255, overlay_g)), 8);
            __m128i pix_b = _mm_cvtps_epi32(_mm_mul_ps(wide_255, overlay_b));

            __m128i pixel_vals =
                _mm_or_si128(pix_a,
                             _mm_or_si128(pix_r, _mm_or_si128(pix_g, pix_b)));
            _mm_storeu_si128(pixel_loc, pixel_vals);

            xs = _mm_add_ps(xs, wide_4);
        }

        for (; x < end_x; ++x) {
            v2 p_d = v2_from_ints(x, y);
            v2 p_s = transform * p_d;

            rgba_t composite = {0};
            for (int i = 0; i < supersampling_vector_count; ++i) {
                v2 p = p_s + supersampling_vectors[i];
                i32 xi = (i32)(p.x);
                i32 yi = (i32)(p.y);
                rgba_t pix = is_in_rect(xi, yi, bmp_rect) ?
                    to_rgba(bitmap.pixels[yi * bitmap.pitch + xi]) : transparent;
                composite.a += pix.a;
                composite.rgb += pix.a * pix.rgb;
            }

            if (!fequals(composite.a, 0.0f)) {
                composite.rgb = (1.0f / composite.a) * composite.rgb;
                composite.a = composite.a / (f32)supersampling_vector_count;
            } else {
                composite.rgb = color_t {0};
            }

            rgba_t existing = to_rgba(pixels[y * buffer.width + x]);

            pixels[y * buffer.width + x] = from_rgba(overlay(existing, composite));

        }
    }    
}

void draw_circ_outline(video_buffer_description_t buffer,
                       rect_i clip_rect,
                       color_t color,
                       v2 center,
                       f32 radius) {

}

// NOTE(doug): orientation is counter-clockwise radians from the x axis
void draw_rectangle(video_buffer_description_t buffer,
                    rect_i clip_rect,
                    color_t color, i32 center_x, i32 center_y, u32 width, u32 height,
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

    start_x = (i32) fmax(clip_rect.min_x, start_x - 1);
    start_y = (i32) fmax(clip_rect.min_y, start_y - 1);
    end_x = (i32) fmin(clip_rect.max_x, fmax(0, end_x + 1));
    end_y = (i32) fmin(clip_rect.max_y, end_y + 1);

    v2 center = v2 {(f32)center_x, (f32)center_y};
    u32* pixels = (u32*)buffer.memory;
    u32 test = pixels[(buffer.width * buffer.height)];

    m3x3 transform = get_translation_matrix(center) *
            inverse_rotate *
            get_translation_matrix(-center);

    __m128 transform_wide[9];

    for (int i = 0; i < 9; ++i) {
        transform_wide[i] = _mm_set1_ps(transform.vals[i]);
    }

    __m128 min_x_wide = _mm_set1_ps(min_x);
    __m128 max_x_wide = _mm_set1_ps(max_x);

    __m128 min_y_wide = _mm_set1_ps(min_y);
    __m128 max_y_wide = _mm_set1_ps(max_y);

    __m128 wide_1 = _mm_set1_ps(1.0f);
    __m128 wide_half = _mm_set1_ps(0.5f);
    __m128 wide_0 = _mm_set1_ps(0.0f);
    __m128 wide_4 = _mm_set1_ps(4.0f);
    __m128 wide_255 = _mm_set1_ps(255.0f);
    __m128 wide_inv255 = _mm_set1_ps(1.0f / 255.0f);

    __m128i a_mask = _mm_set1_epi32(0xff000000);
    __m128i r_mask = _mm_set1_epi32(0x00ff0000);
    __m128i g_mask = _mm_set1_epi32(0x0000ff00);
    __m128i b_mask = _mm_set1_epi32(0x000000ff);

    __m128 wide_r = _mm_mul_ps(_mm_set1_ps(color.r), wide_255);
    __m128 wide_g = _mm_mul_ps(_mm_set1_ps(color.g), wide_255);;
    __m128 wide_b = _mm_mul_ps(_mm_set1_ps(color.b), wide_255);;

    __m128 xs;
    {
		TIMED_FUNC();
		for (int y = start_y; y < end_y; ++y) {
    
            ((f32*)&xs)[0] = (f32)(start_x + 0);
            ((f32*)&xs)[1] = (f32)(start_x + 1);
            ((f32*)&xs)[2] = (f32)(start_x + 2);
            ((f32*)&xs)[3] = (f32)(start_x + 3);
    
            __m128 ys = _mm_set1_ps((f32)y);
            __m128 first_transformed_ys = 
                _mm_add_ps(_mm_mul_ps(transform_wide[1], ys),
                           transform_wide[2]);
            __m128 second_transformed_ys = 
                _mm_add_ps(_mm_mul_ps(transform_wide[4], ys),
                           transform_wide[5]);
    
            i32 x;
            for (x = start_x; x < end_x-4; x += 4) {

                __m128 p_original_xs =
                    _mm_add_ps(_mm_mul_ps(transform_wide[0], xs), first_transformed_ys);
                __m128 p_original_ys =
                    _mm_add_ps(_mm_mul_ps(transform_wide[3], xs), second_transformed_ys);
    
                __m128 diff_xs = 
                    wide_max(_mm_sub_ps(min_x_wide, p_original_xs),
                             _mm_sub_ps(p_original_xs, max_x_wide));
    
                __m128 diff_ys = 
                    wide_max(_mm_sub_ps(min_y_wide, p_original_ys),
                             _mm_sub_ps(p_original_ys, max_y_wide));
    
                diff_xs = wide_clamp(_mm_add_ps(diff_xs, wide_half), wide_0, wide_1);
                diff_ys = wide_clamp(_mm_add_ps(diff_ys, wide_half), wide_0, wide_1);
    
                __m128 ratios = _mm_mul_ps(_mm_sub_ps(wide_1, diff_xs),
                                          _mm_sub_ps(wide_1, diff_ys));
    
                __m128 one_minus_ratios = _mm_sub_ps(wide_1, ratios);
    
                __m128i* pixel_loc = (__m128i*)(pixels + (y * buffer.width + x));
    
                __m128i existing_i = _mm_loadu_si128((__m128i*)pixel_loc);
                __m128i existing_r = _mm_srli_epi32(_mm_and_si128(existing_i, r_mask), 16);
                __m128i existing_g = _mm_srli_epi32(_mm_and_si128(existing_i, g_mask), 8);
                __m128i existing_b = _mm_and_si128(existing_i, b_mask);
    
                __m128 fexisting_r = _mm_cvtepi32_ps(existing_r);
                __m128 fexisting_g = _mm_cvtepi32_ps(existing_g);
                __m128 fexisting_b = _mm_cvtepi32_ps(existing_b);
    
                __m128 new_r = _mm_mul_ps(one_minus_ratios, fexisting_r);
                __m128 new_g = _mm_mul_ps(one_minus_ratios, fexisting_g);
                __m128 new_b = _mm_mul_ps(one_minus_ratios, fexisting_b);
                new_r = _mm_add_ps(new_r, _mm_mul_ps(ratios, wide_r));
                new_g = _mm_add_ps(new_g, _mm_mul_ps(ratios, wide_g));
                new_b = _mm_add_ps(new_b, _mm_mul_ps(ratios, wide_b));
    
                existing_r = _mm_slli_epi32(_mm_cvtps_epi32(new_r), 16);
                existing_g = _mm_slli_epi32(_mm_cvtps_epi32(new_g), 8);
                existing_b = _mm_cvtps_epi32(new_b);
    
                __m128i pixel_vals =
                    _mm_or_si128(a_mask,
                                 _mm_or_si128(existing_r, _mm_or_si128(existing_g, existing_b)));
                _mm_storeu_si128(pixel_loc, pixel_vals);
    
                xs = _mm_add_ps(wide_4, xs);
            }
    
            for (; x < end_x; ++x) {
                v2 p_original = transform * v2 { (f32)x, (f32)y };
                f32 diff_x = fmax(min_x - p_original.x, p_original.x - max_x);
                f32 diff_y = fmax(min_y - p_original.y, p_original.y - max_y);
    
                diff_x = fclamp(diff_x + 0.5f, 0.0f, 1.0f);
                diff_y = fclamp(diff_y + 0.5f, 0.0f, 1.0f);
    
                f32 ratio = (1.0f - diff_x) * (1.0f - diff_y);
    
                v3 existing = to_rgb(pixels[y * buffer.width + x]);
                pixels[y * buffer.width + x] =
                        from_rgb((1.0f - ratio) * existing + ratio * color);
    
            }
        }
    }
}

global i32 debug_point_count = 0;
global v2 debug_points[200];
global i32 debug_colors[200];

void add_debug_point(v2 point, i32 color) {
    debug_points[debug_point_count] = point;
    debug_colors[debug_point_count++] = color;
}

global i32 debug_rect_count = 0;
global v2 debug_rects[20000];
global i32 debug_rect_colors[10000];

void add_debug_rect(v2 top_right, v2 bottom_left, i32 color) {
    debug_rects[debug_rect_count] = top_right;
    debug_rects[debug_rect_count+1] = bottom_left;
    debug_colors[debug_rect_count / 2] = color;
    debug_rect_count += 2;
}
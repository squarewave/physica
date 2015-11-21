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
#include "stb_truetype.h"
#include "stb_image.h"

i32 check_shader_status(u32 item_id, GLenum status_id) {
    i32 result = GL_FALSE;
    i32 info_log_length = 0;
    glGetShaderiv(item_id, status_id, &result);
    if (result != GL_TRUE) {
        glGetShaderiv(item_id, GL_INFO_LOG_LENGTH, &info_log_length);
        char* buffer = (char*)alloca(info_log_length);
        glGetShaderInfoLog(item_id, info_log_length, NULL, buffer);
        OutputDebugString(buffer);
    }

    return result;
}

i32 check_program_status(u32 item_id, GLenum status_id) {
    i32 result = GL_FALSE;
    i32 info_log_length = 0;
    glGetProgramiv(item_id, status_id, &result);
    if (result != GL_TRUE) {
        glGetProgramiv(item_id, GL_INFO_LOG_LENGTH, &info_log_length);
        char* buffer = (char*)alloca(info_log_length);
        glGetProgramInfoLog(item_id, info_log_length, NULL, buffer);
        OutputDebugString(buffer);
    }

    return result;
}

b32 load_program(u32* program_result,
                 const char* vertex_path,
                 const char* fragment_path) {
    platform_read_entire_file_result_t vertex_file = platform_read_entire_file(vertex_path);
    char* vertex_code = (char*)vertex_file.contents;
    char* fragment_code = (char*)platform_read_entire_file(fragment_path).contents;

    u32 program_id = glCreateProgram();
    u32 vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    u32 fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertex_shader_id, 1, &vertex_code, 0);
    glCompileShader(vertex_shader_id);
    if (check_shader_status(vertex_shader_id, GL_COMPILE_STATUS) != GL_TRUE) {
        return false;
    }
    glAttachShader(program_id, vertex_shader_id);

    glShaderSource(fragment_shader_id, 1, &fragment_code, 0);
    glCompileShader(fragment_shader_id);
    if (check_shader_status(fragment_shader_id, GL_COMPILE_STATUS) != GL_TRUE) {
        return false;
    }
    glAttachShader(program_id, fragment_shader_id);

    glLinkProgram(program_id);
    if (check_program_status(program_id, GL_LINK_STATUS) != GL_TRUE) {
        return false;
    }

    *program_result = program_id;

    // glDeleteShader(vertex_shader_id);
    // glDeleteShader(fragment_shader_id);

    return true;
}

gl_programs_t load_programs() {
    gl_programs_t result;

    gl_program_t solid_particles_program;
    glGenVertexArrays(1,&solid_particles_program.vaos[GL_VAO_RECT]);
    glBindVertexArray(solid_particles_program.vaos[GL_VAO_RECT]);

    if (!load_program(&solid_particles_program.id,
                      "shaders/simple_particle_vertex_shader.glsl",
                      "shaders/simple_particle_fragment_shader.glsl")) {
        OutputDebugString("Error loading GL program");
        assert_(false);
    }
    solid_particles_program.attribs[GL_ATTRIB_VERTEX_MODELSPACE] =
        glGetAttribLocation(solid_particles_program.id, "modelspace");
    solid_particles_program.attribs[GL_ATTRIB_PARTICLE_CENTER] =
        glGetAttribLocation(solid_particles_program.id, "center");
    solid_particles_program.attribs[GL_ATTRIB_PARTICLE_SCALING] =
        glGetAttribLocation(solid_particles_program.id, "scaling");
    solid_particles_program.attribs[GL_ATTRIB_PARTICLE_DRAW_COLOR] =
        glGetAttribLocation(solid_particles_program.id, "draw_color");

    GLfloat solid_particles_vertex_data[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };
    u32 solid_particles_index_data[] = { 0, 1, 2, 3 };

    u32 solid_particle_vbo;
    u32 solid_particle_ibo;
    glGenBuffers(1, &solid_particle_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, solid_particle_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(solid_particles_vertex_data),
                 solid_particles_vertex_data,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(solid_particles_program.attribs[GL_ATTRIB_VERTEX_MODELSPACE],
                          3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(solid_particles_program.attribs[GL_ATTRIB_VERTEX_MODELSPACE]);


    glGenBuffers(1, &solid_particles_program.dynamic_buffers[GL_DYNAMIC_CENTERS]);
    glBindBuffer(GL_ARRAY_BUFFER,
                 solid_particles_program.dynamic_buffers[GL_DYNAMIC_CENTERS]);
    glBufferData(GL_ARRAY_BUFFER,
                 MAX_PARTICLES * sizeof(f32) * 4,
                 0,
                 GL_STREAM_DRAW);
    glVertexAttribPointer(solid_particles_program.attribs[GL_ATTRIB_PARTICLE_CENTER],
                          4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(solid_particles_program.attribs[GL_ATTRIB_PARTICLE_CENTER]);
    glVertexAttribDivisor(solid_particles_program.attribs[GL_ATTRIB_PARTICLE_CENTER], 1);

    glGenBuffers(1, &solid_particles_program.dynamic_buffers[GL_DYNAMIC_SCALING]);
    glBindBuffer(GL_ARRAY_BUFFER,
                 solid_particles_program.dynamic_buffers[GL_DYNAMIC_SCALING]);
    glBufferData(GL_ARRAY_BUFFER,
                 MAX_PARTICLES * sizeof(f32) * 2,
                 0,
                 GL_STREAM_DRAW);
    glVertexAttribPointer(solid_particles_program.attribs[GL_ATTRIB_PARTICLE_SCALING],
                          2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(solid_particles_program.attribs[GL_ATTRIB_PARTICLE_SCALING]);
    glVertexAttribDivisor(solid_particles_program.attribs[GL_ATTRIB_PARTICLE_SCALING], 1);

    glGenBuffers(1, &solid_particles_program.dynamic_buffers[GL_DYNAMIC_DRAW_COLOR]);
    glBindBuffer(GL_ARRAY_BUFFER,
                 solid_particles_program.dynamic_buffers[GL_DYNAMIC_DRAW_COLOR]);
    glBufferData(GL_ARRAY_BUFFER,
                 MAX_PARTICLES * sizeof(f32) * 4,
                 0,
                 GL_STREAM_DRAW);
    glVertexAttribPointer(solid_particles_program.attribs[GL_ATTRIB_PARTICLE_DRAW_COLOR],
                          4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(solid_particles_program.attribs[GL_ATTRIB_PARTICLE_DRAW_COLOR]);
    glVertexAttribDivisor(solid_particles_program.attribs[GL_ATTRIB_PARTICLE_DRAW_COLOR], 1);


    glGenBuffers(1, &solid_particle_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, solid_particle_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(solid_particles_index_data),
                 solid_particles_index_data,
                 GL_STATIC_DRAW);

    glGenVertexArrays(1,&solid_particles_program.vaos[GL_VAO_CIRCLE]);
    glBindVertexArray(solid_particles_program.vaos[GL_VAO_CIRCLE]);

    solid_particles_program.uniforms[GL_UNIFORM_MAIN_TRANSFORM] =
        glGetUniformLocation(solid_particles_program.id, "view_transform");

    result.programs[GL_PROG_SOLID_PARTICLES] = solid_particles_program;


    // solids program:
    // for rendering single solid objects of one color like rects and circles
    gl_program_t solids_program;
    glGenVertexArrays(1,&solids_program.vaos[GL_VAO_RECT]);
    glBindVertexArray(solids_program.vaos[GL_VAO_RECT]);

    if (!load_program(&solids_program.id,
                      "shaders/simple_vertex_shader.glsl",
                      "shaders/simple_fragment_shader.glsl")) {
        OutputDebugString("Error loading GL program");
        assert_(false);
    }
    solids_program.attribs[GL_ATTRIB_VERTEX_MODELSPACE] =
        glGetAttribLocation(solids_program.id, "vertex_modelspace");

    GLfloat solids_rect_vertex_data[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };
    u32 solids_rect_index_data[] = { 0, 1, 2, 3 };

    u32 rect_vbo;
    u32 rect_ibo;
    u32 circle_vbo;
    u32 circle_ibo;
    glGenBuffers(1, &rect_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(solids_rect_vertex_data),
                 solids_rect_vertex_data,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(solids_program.attribs[GL_ATTRIB_VERTEX_MODELSPACE],
                          3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(solids_program.attribs[GL_ATTRIB_VERTEX_MODELSPACE]);

    glGenBuffers(1, &rect_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rect_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(solids_rect_index_data),
                 solids_rect_index_data,
                 GL_STATIC_DRAW);

    glGenVertexArrays(1,&solids_program.vaos[GL_VAO_CIRCLE]);
    glBindVertexArray(solids_program.vaos[GL_VAO_CIRCLE]);

    u32 circle_index_data[25] = {0};
    GLfloat circle_vertex_data[ARRAY_SIZE(circle_index_data) * 3] = {0};

    circle_index_data[0] = 0;
    circle_vertex_data[0] = 0.0f;
    circle_vertex_data[1] = 0.0f;
    circle_vertex_data[2] = 0.0f;

    f32 angle = f2PI / ((f32)ARRAY_SIZE(circle_index_data) - 2); 
    for (int i = 1; i < ARRAY_SIZE(circle_index_data); ++i) {
        circle_vertex_data[i*3] = sin(angle * (f32)(i-1));
        circle_vertex_data[i*3 + 1] = cos(angle * (f32)(i-1));
        circle_vertex_data[i*3 + 2] = 0.0f;
        circle_index_data[i] = i;
    }

    glGenBuffers(1, &circle_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, circle_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(circle_vertex_data),
                 circle_vertex_data,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(solids_program.attribs[GL_ATTRIB_VERTEX_MODELSPACE],
                          3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(solids_program.attribs[GL_ATTRIB_VERTEX_MODELSPACE]);

    glGenBuffers(1, &circle_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circle_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(circle_index_data),
                 circle_index_data,
                 GL_STATIC_DRAW);

    solids_program.uniforms[GL_UNIFORM_MAIN_TRANSFORM] =
        glGetUniformLocation(solids_program.id, "transform");
    solids_program.uniforms[GL_UNIFORM_DRAW_COLOR] =
        glGetUniformLocation(solids_program.id, "draw_color");

    result.programs[GL_PROG_SOLIDS] = solids_program;


    gl_program_t texture_program;

    glGenVertexArrays(1,&texture_program.vaos[GL_VAO_RECT]);
    glBindVertexArray(texture_program.vaos[GL_VAO_RECT]);

    if (!load_program(&texture_program.id,
                      "shaders/texture_vertex_shader.glsl",
                      "shaders/texture_fragment_shader.glsl")) {
        OutputDebugString("Error loading GL program");
        assert_(false); 
    }
    
    texture_program.attribs[GL_ATTRIB_VERTEX_MODELSPACE] =
        glGetAttribLocation(texture_program.id, "vertex_modelspace");
    texture_program.attribs[GL_ATTRIB_VERTEX_UV] =
        glGetAttribLocation(texture_program.id, "vertex_uv");

    GLfloat texture_vertex_data[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };

    GLfloat texture_uv_data[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    u32 texture_index_data[] = { 0, 1, 2, 3 };

    u32 texture_vbo;
    u32 texture_uv_vbo;
    u32 texture_ibo;

    glGenBuffers(1, &texture_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, texture_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(texture_vertex_data),
                 texture_vertex_data,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(texture_program.attribs[GL_ATTRIB_VERTEX_MODELSPACE]);
    glVertexAttribPointer(texture_program.attribs[GL_ATTRIB_VERTEX_MODELSPACE],
                          3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &texture_uv_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, texture_uv_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(texture_uv_data),
                 texture_uv_data,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(texture_program.attribs[GL_ATTRIB_VERTEX_UV]);
    glVertexAttribPointer(texture_program.attribs[GL_ATTRIB_VERTEX_UV],
                          2, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &texture_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texture_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(texture_index_data),
                 texture_index_data,
                 GL_STATIC_DRAW);

    texture_program.uniforms[GL_UNIFORM_MAIN_TRANSFORM] =
        glGetUniformLocation(texture_program.id, "texture_transform");
    texture_program.uniforms[GL_UNIFORM_UV_TRANSFORM] =
        glGetUniformLocation(texture_program.id, "uv_transform");
    texture_program.uniforms[GL_UNIFORM_SAMPLER] =
        glGetUniformLocation(texture_program.id, "texture_sampler");
    texture_program.uniforms[GL_UNIFORM_DRAW_COLOR] =
        glGetUniformLocation(texture_program.id, "tint");

    result.programs[GL_PROG_TEXTURES] = texture_program;

    return result;
}

frame_buffer_t default_frame_buffer(i32 width, i32 height) {
    frame_buffer_t result;

    result.width = width;
    result.height = height;
    result.id = 0;
    result.texture = 0;

    return result;
}

frame_buffer_t create_frame_buffer(i32 width, i32 height) {
    frame_buffer_t result;

    result.width = width;
    result.height = height;

    glGenFramebuffers(1, &result.id);
    glBindFramebuffer(GL_FRAMEBUFFER, result.id);

    glGenTextures(1, &result.texture);
    glBindTexture(GL_TEXTURE_2D, result.texture);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 width,
                 height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_INT_8_8_8_8,
                 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           result.texture,
                           0);


    u32 depth_buffer;
    glGenRenderbuffers(1, &depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER,
                          GL_DEPTH_COMPONENT,
                          width,
                          height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER,
                              depth_buffer);

    return result;
}

tex2 load_image(char* filename) {
    tex2 result;

    stbi_set_flip_vertically_on_load(true);

    platform_read_entire_file_result_t file = platform_read_entire_file(filename);

    i32 x;
    i32 y;
    i32 n;
    i32 num_channels = 4;
    u8* bitmap = stbi_load_from_memory(file.contents,
                                       file.content_size,
                                       &x,
                                       &y,
                                       &n,
                                       num_channels);

    result.pixels = (u32*)bitmap;
    result.width = x;
    result.height = y;
    result.pitch = x;

    glGenTextures(1, &result.texture_id);
    glBindTexture(GL_TEXTURE_2D, result.texture_id);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 result.width,
                 result.height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_INT_8_8_8_8_REV,
                 (void*)result.pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    return result;

}

// this is really inefficient and is only for internal tools
glyph_spec_t load_glyph(char* font_file_name, char c, f32 pixel_height) {

    glyph_spec_t result;
    tex2 texture;
    stbtt_fontinfo font;

    platform_read_entire_file_result_t file =
        platform_read_entire_file(font_file_name);

    i32 width;
    i32 height;
    i32 xoff;
    i32 yoff;
    stbtt_InitFont(&font, file.contents, stbtt_GetFontOffsetForIndex(file.contents,0));

    f32 scale = stbtt_ScaleForPixelHeight(&font, pixel_height);
    u8* bitmap = stbtt_GetCodepointBitmap(&font,
                                          0,
                                          scale,
                                          c,
                                          &width,
                                          &height,
                                          &xoff,
                                          &yoff);

    result.hotspot = v2 {(f32)xoff, (f32)height-yoff};

    i32 advance_width;
    i32 left_side_bearing;
    stbtt_GetCodepointHMetrics(&font, c, &advance_width, &left_side_bearing);
    result.advance_width = scale * (f32)advance_width;
    result.left_side_bearing = scale * (f32)left_side_bearing;

    platform_free_file_memory(file.contents);

    texture.width = width;
    texture.height = height;
    texture.pitch = width;
    texture.pixels = (u32*)malloc(width * height * sizeof(u32));

    for (i32 i = 0; i < height; ++i) {
        for (i32 j = 0; j < width; ++j) {
            texture.pixels[i * width + j] = 0xffffff00 | bitmap[(height - i - 1) * width + j];
        }
    }

    glGenTextures(1, &texture.texture_id);
    glBindTexture(GL_TEXTURE_2D, texture.texture_id);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 texture.width,
                 texture.height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_INT_8_8_8_8,
                 (void*)texture.pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    stbtt_FreeBitmap(bitmap, 0);

    result.texture = texture;
    return result;
}

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
                           b32 parallax) {
    i32 i = render_group->objects.push_unassigned();
    render_object_t* obj = render_group->objects.at(i);
    obj->type = RENDER_TYPE_RECT;
    obj->render_rect.color = color;
    obj->render_rect.center = center;
    obj->render_rect.diagonal = diagonal;
    obj->render_rect.orientation = orientation;
    obj->render_rect.parallax = parallax;
    obj->z = z;
    return obj;
}

render_object_t* push_texture(render_group_t* render_group,
                              v2 center,
                              v2 hotspot,
                              f32 pixel_size,
                              tex2 texture,
                              rect_i source_rect,
                              rgba_t tint,
                              f32 orientation,
                              f32 z,
                              b32 parallax) {
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
    obj->render_texture.parallax = parallax;
    obj->render_texture.tint = tint;
    return obj;
}

void swap_render_objs(render_object_t* a, render_object_t* b) {
    render_object_t tmp = *a;
    *a = *b;
    *b = tmp;
}

void sort_render_objects(vec<render_object_t> objs) {
    TIMED_FUNC();
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

void draw_gl_rect(gl_program_t* program,
                  camera_t camera,
                  render_rect_t rect,
                  f32 z,
                  u32 flags) {
    TIMED_FUNC();
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

    glUniformMatrix4fv(program->uniforms[GL_UNIFORM_MAIN_TRANSFORM],
                       1,
                       GL_TRUE,
                       transform.vals);
    glUniform4f(program->uniforms[GL_UNIFORM_DRAW_COLOR],
                rect.color.r,
                rect.color.g,
                rect.color.b,
                1.0f);
    if (flags & RENDER_WIREFRAME) {
        glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0);
    } else {
        glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);
    }
}

void draw_gl_circle(gl_program_t* program,
                    camera_t camera,
                    render_circle_t circle,
                    f32 z,
                    u32 flags) {
    TIMED_FUNC();
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

    glUniformMatrix4fv(program->uniforms[GL_UNIFORM_MAIN_TRANSFORM],
                       1,
                       GL_TRUE,
                       transform.vals);
    glUniform4f(program->uniforms[GL_UNIFORM_DRAW_COLOR],
                circle.color.r,
                circle.color.g,
                circle.color.b,
                1.0f);

    i32 vertex_count = 0;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &vertex_count);

    if (flags & RENDER_WIREFRAME) {
        glDrawArrays(GL_LINE_STRIP, 1, vertex_count  - 1);
    } else {
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
    }
}

void draw_gl_texture(gl_program_t* program,
                     camera_t camera,
                     render_texture_t texture,
                     f32 z) {
    TIMED_FUNC();

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

    m4x4 view = get_parallaxed_view_transform_4x4(camera,
                                                  texture.parallax ? (1.0f - z) : 1.0f);

    m4x4 transform = view * model;

    m3x3 uv_transform = identity_3x3();
    uv_transform.r1.c1 = src_width / (f32)texture.texture.width;
    uv_transform.r2.c2 = src_height / (f32)texture.texture.height;
    uv_transform.r1.c3 = texture.source_rect.min_x / (f32)texture.texture.width;
    uv_transform.r2.c3 = texture.source_rect.min_y / (f32)texture.texture.height;

    glBindTexture(GL_TEXTURE_2D, texture.texture.texture_id);
    glUniform1i(program->uniforms[GL_UNIFORM_SAMPLER], 0);

    glUniform4f(program->uniforms[GL_UNIFORM_DRAW_COLOR],
                texture.tint.r,
                texture.tint.g,
                texture.tint.b,
                texture.tint.a);

    glUniformMatrix4fv(program->uniforms[GL_UNIFORM_MAIN_TRANSFORM],
                       1,
                       GL_TRUE,
                       transform.vals);
    glUniformMatrix3fv(program->uniforms[GL_UNIFORM_UV_TRANSFORM],
                       1,
                       GL_TRUE,
                       uv_transform.vals);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);
}

void setup_gl_for_type(gl_programs_t* programs,
                       u32 type) {

    switch (type) {
        case RENDER_TYPE_RECT: {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glUseProgram(programs->programs[GL_PROG_SOLIDS].id);
            glBindVertexArray(programs->programs[GL_PROG_SOLIDS].vaos[GL_VAO_RECT]);
        } break;
        case RENDER_TYPE_CIRCLE: {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glUseProgram(programs->programs[GL_PROG_SOLIDS].id);
        } break;
        case RENDER_TYPE_TEXTURE: {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glUseProgram(programs->programs[GL_PROG_TEXTURES].id);
            glBindVertexArray(programs->programs[GL_PROG_TEXTURES].vaos[GL_VAO_RECT]);
            glActiveTexture(GL_TEXTURE0);
        } break;
        case RENDER_TYPE_RECT_OUTLINE: {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glUseProgram(programs->programs[GL_PROG_SOLIDS].id);
            glBindVertexArray(programs->programs[GL_PROG_SOLIDS].vaos[GL_VAO_RECT]);
        } break;
        case RENDER_TYPE_CIRCLE_OUTLINE: {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glUseProgram(programs->programs[GL_PROG_SOLIDS].id);
            glBindVertexArray(programs->programs[GL_PROG_SOLIDS].vaos[GL_VAO_CIRCLE]);
        } break;
    }
}

void setup_frame_buffer(frame_buffer_t frame_buffer) {
    TIMED_FUNC();

    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer.id);
    glViewport(0,0,frame_buffer.width, frame_buffer.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void present_frame_buffer(gl_programs_t* programs,
                          frame_buffer_t source,
                          frame_buffer_t dest) {
    TIMED_FUNC();

    setup_gl_for_type(programs, RENDER_TYPE_TEXTURE);

    gl_program_t* program = &programs->programs[GL_PROG_TEXTURES];

    glBindFramebuffer(GL_FRAMEBUFFER, dest.id);
    glViewport(0,0,dest.width, dest.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, source.texture);
    glUniform1i(program->uniforms[GL_UNIFORM_SAMPLER], 0);

    m4x4 transform = identity_4x4();
    transform.r1.c1 = 2.0f;
    transform.r2.c2 = 2.0f;
    m3x3 uv_transform = identity_3x3();
    glUniformMatrix4fv(program->uniforms[GL_UNIFORM_MAIN_TRANSFORM],
                       1,
                       GL_TRUE,
                       transform.vals);
    glUniformMatrix3fv(program->uniforms[GL_UNIFORM_UV_TRANSFORM],
                       1,
                       GL_TRUE,
                       uv_transform.vals);

    glUniform4f(program->uniforms[GL_UNIFORM_DRAW_COLOR],
                0.0f,
                0.0f,
                0.0f,
                0.0f);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);

}

void draw_render_group(gl_programs_t* programs,
                       camera_t camera,
                       render_group_t* render_group) {
    TIMED_FUNC();

    f32 max_distance_sq = length_squared(2.0f * camera.to_top_right);

    u32 types[] = {
        RENDER_TYPE_RECT,
        RENDER_TYPE_CIRCLE,
        RENDER_TYPE_TEXTURE,
        RENDER_TYPE_RECT_OUTLINE,
        RENDER_TYPE_CIRCLE_OUTLINE
    };

    i32 particle_count = 0;
    v4 particle_center_data[MAX_PARTICLES];
    v2 particle_scaling_data[MAX_PARTICLES];
    rgba_t particle_color_data[MAX_PARTICLES];

    for (int i = 0; i < ARRAY_SIZE(types); ++i) {
        setup_gl_for_type(programs, types[i]);
        for (int j = 0; j < render_group->objects.count; ++j) {
            render_object_t* obj = render_group->objects.at(j);

            if (obj->type != types[i]) {
                continue;
            }

            f32 parallax = obj->parallax ? (1.0f - obj->z) : 1.0f;
            if (length_squared(camera.center * parallax - obj->center) > max_distance_sq) {
                continue;
            }

            switch (types[i]) {
                case RENDER_TYPE_RECT_OUTLINE: 
                case RENDER_TYPE_RECT: {
                    particle_center_data[particle_count] = 
                        v4 {obj->center.x, obj->center.y, obj->z, parallax};
                    color_t color = obj->render_rect.color;
                    particle_color_data[particle_count] =
                        rgba_t {color.r,color.g,color.b,1.0f};
                    particle_scaling_data[particle_count] =
                        v2 {obj->render_rect.diagonal};

                    particle_count++;
                    // draw_gl_rect(&programs->programs[GL_PROG_SOLIDS],
                    //              camera,
                    //              obj->render_rect,
                    //              obj->z,
                    //              obj->flags);
                } break;
                case RENDER_TYPE_CIRCLE_OUTLINE:
                case RENDER_TYPE_CIRCLE: {
                    draw_gl_circle(&programs->programs[GL_PROG_SOLIDS],
                                   camera,
                                   obj->render_circle,
                                   obj->z,
                                   obj->flags);
                } break;
                case RENDER_TYPE_TEXTURE: {
                    draw_gl_texture(&programs->programs[GL_PROG_TEXTURES],
                                    camera,
                                    obj->render_texture,
                                    obj->z);
                } break;
            }
        }
    }

    gl_program_t* particles_program = &programs->programs[GL_PROG_SOLID_PARTICLES];

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUseProgram(particles_program->id);
    glBindVertexArray(particles_program->vaos[GL_VAO_RECT]);

    glBindBuffer(GL_ARRAY_BUFFER,
                 particles_program->dynamic_buffers[GL_DYNAMIC_CENTERS]);
    glBufferData(GL_ARRAY_BUFFER,
                 MAX_PARTICLES * sizeof(*particle_center_data),
                 0,
                 GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    particle_count * sizeof(*particle_center_data),
                    particle_center_data);

    glBindBuffer(GL_ARRAY_BUFFER,
                 particles_program->dynamic_buffers[GL_DYNAMIC_SCALING]);
    glBufferData(GL_ARRAY_BUFFER,
                 MAX_PARTICLES * sizeof(*particle_scaling_data),
                 0,
                 GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    particle_count * sizeof(*particle_scaling_data),
                    particle_scaling_data);

    glBindBuffer(GL_ARRAY_BUFFER,
                 particles_program->dynamic_buffers[GL_DYNAMIC_DRAW_COLOR]);
    glBufferData(GL_ARRAY_BUFFER,
                 MAX_PARTICLES * sizeof(*particle_color_data),
                 0,
                 GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    particle_count * sizeof(*particle_color_data),
                    particle_color_data);

    m4x4 view_transform = get_view_transform_4x4(camera);

    glUniformMatrix4fv(particles_program->uniforms[GL_UNIFORM_MAIN_TRANSFORM],
                       1,
                       GL_TRUE,
                       view_transform.vals);

    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, particle_count);
}

void clear_render_group(render_group_t* render_group) {
    render_group->objects.count = 0;
}
//
// Created by doug on 4/16/15.
//

#include <assert.h>
#include "physica_math.h"
#include "renderer.h"
#include "game_intrinsics.h"
#include "game.h"
#include "GL/gl.h"
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
        platform_debug_print(buffer);
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
        platform_debug_print(buffer);
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
#if 0
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
    RES_TEXTURES_TEXTURE_ID,

    RES_SOLID_PARTICLES_PROG,
    RES_SOLID_PARTICLES_VAO_RECT,
    RES_SOLID_PARTICLES_VERTEX_MODELSPACE,
    RES_SOLID_PARTICLES_CENTER,
    RES_SOLID_PARTICLES_SCALING,
    RES_SOLID_PARTICLES_DRAW_COLOR,
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

    RES_SOLID_PARTICLES_CENTER_BUFFER,
    RES_SOLID_PARTICLES_SCALING_BUFFER,
    RES_SOLID_PARTICLES_DRAW_COLOR_BUFFER,

    RES_COLORED_VERTEX_PROG,
    RES_COLORED_VERTEX_VAO_RECT,
    RES_COLORED_VERTEX_VERTEX_MODELSPACE,
    RES_COLORED_VERTEX_DRAW_COLOR,

    RES_COUNT,
};
#endif

gl_programs_t load_programs() {
    gl_programs_t result;
    i32* res = result.i_res_ids;
    u32* ures = result.u_res_ids;

    /////////////////////////////////////////////////////////////
    // gradient program
    /////////////////////////////////////////////////////////////

    {
        if (!load_program(&ures[RES_GRADIENT_PROG],
                          "shaders/passthrough_vertex_shader.glsl",
                          "shaders/gradient_fragment_shader.glsl")) {
            platform_debug_print("Error loading GL program");
            assert_(false);
        }

        glGenVertexArrays(1,&ures[RES_GRADIENT_VAO_RECT]);
        glBindVertexArray(ures[RES_GRADIENT_VAO_RECT]);

        res[RES_GRADIENT_VERTEX_MODELSPACE] = glGetAttribLocation(res[RES_GRADIENT_PROG],
                                                                  "vertex_modelspace");
        res[RES_GRADIENT_VIEWPORT] = glGetUniformLocation(res[RES_GRADIENT_PROG],
                                                          "viewport");
        res[RES_GRADIENT_START_COLOR] = glGetUniformLocation(res[RES_GRADIENT_PROG],
                                                             "start_color");
        res[RES_GRADIENT_END_COLOR] = glGetUniformLocation(res[RES_GRADIENT_PROG],
                                                           "end_color");
        res[RES_GRADIENT_GRADIENT_START] = glGetUniformLocation(res[RES_GRADIENT_PROG],
                                                                "gradient_start");
        res[RES_GRADIENT_GRADIENT_END] = glGetUniformLocation(res[RES_GRADIENT_PROG],
                                                              "gradient_end");
        f32 vertex_data[] = {
            -1.0f, -1.0f, 0.99f,
             1.0f, -1.0f, 0.99f,
             1.0f,  1.0f, 0.99f,
            -1.0f,  1.0f, 0.99f
        };
        u32 index_data[] = { 0, 1, 2, 3 };

        u32 vbo;
        u32 ibo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vertex_data),
                     vertex_data,
                     GL_STATIC_DRAW);
        glVertexAttribPointer(res[RES_GRADIENT_VERTEX_MODELSPACE],
                              3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(res[RES_GRADIENT_VERTEX_MODELSPACE]);

        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(index_data),
                     index_data,
                     GL_STATIC_DRAW);
    }

    /////////////////////////////////////////////////////////////
    // colored vertex program
    /////////////////////////////////////////////////////////////

    {
        if (!load_program(&ures[RES_COLOR_PICKER_PROG],
                          "shaders/simple_vertex_shader.glsl",
                          "shaders/color_picker_fragment_shader.glsl")) {
            platform_debug_print("Error loading GL program");
            assert_(false);
        }

        glGenVertexArrays(1,&ures[RES_COLOR_PICKER_VAO_RECT]);
        glBindVertexArray(ures[RES_COLOR_PICKER_VAO_RECT]);

        res[RES_COLOR_PICKER_HSV] =
            glGetUniformLocation(ures[RES_COLOR_PICKER_PROG], "hsv");
        res[RES_COLOR_PICKER_MIN_P] =
            glGetUniformLocation(ures[RES_COLOR_PICKER_PROG], "min_p");
        res[RES_COLOR_PICKER_MAX_P] =
            glGetUniformLocation(ures[RES_COLOR_PICKER_PROG], "max_p");
        res[RES_COLOR_PICKER_TRANSFORM] =
            glGetUniformLocation(ures[RES_COLOR_PICKER_PROG], "transform");
    
        GLfloat vertex_data[] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f
        };
        u32 index_data[] = { 0, 1, 2, 3 };
    
        u32 vbo;
        u32 ibo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vertex_data),
                     vertex_data,
                     GL_STATIC_DRAW);
        glVertexAttribPointer(res[RES_COLOR_PICKER_VERTEX_MODELSPACE],
                              3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(res[RES_COLOR_PICKER_VERTEX_MODELSPACE]);
    
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(index_data),
                     index_data,
                     GL_STATIC_DRAW);
    }

    /////////////////////////////////////////////////////////////
    // solid particles program
    /////////////////////////////////////////////////////////////

    {
        if (!load_program(&ures[RES_SOLID_PARTICLES_PROG],
                          "shaders/simple_particle_vertex_shader.glsl",
                          "shaders/simple_particle_fragment_shader.glsl")) {
            platform_debug_print("Error loading GL program");
            assert_(false);
        }

        glGenVertexArrays(1,&ures[RES_SOLID_PARTICLES_VAO_RECT]);
        glBindVertexArray(ures[RES_SOLID_PARTICLES_VAO_RECT]);

        res[RES_SOLID_PARTICLES_VERTEX_MODELSPACE] =
            glGetAttribLocation(ures[RES_SOLID_PARTICLES_PROG], "modelspace");
        res[RES_SOLID_PARTICLES_CENTER] =
            glGetAttribLocation(ures[RES_SOLID_PARTICLES_PROG], "center");
        res[RES_SOLID_PARTICLES_SCALING] =
            glGetAttribLocation(ures[RES_SOLID_PARTICLES_PROG], "scaling");
        res[RES_SOLID_PARTICLES_DRAW_COLOR] =
            glGetAttribLocation(ures[RES_SOLID_PARTICLES_PROG], "draw_color");
        res[RES_SOLID_PARTICLES_VIEW_TRANSFORM] =
            glGetUniformLocation(ures[RES_SOLID_PARTICLES_PROG], "view_transform");
        res[RES_SOLID_PARTICLES_LIGHTING] =
            glGetUniformLocation(ures[RES_SOLID_PARTICLES_PROG], "lighting");
    
        GLfloat vertex_data[] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f
        };
        u32 index_data[] = { 0, 1, 2, 3 };
    
        u32 vbo;
        u32 ibo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vertex_data),
                     vertex_data,
                     GL_STATIC_DRAW);
        glVertexAttribPointer(res[RES_SOLID_PARTICLES_VERTEX_MODELSPACE],
                              3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(res[RES_SOLID_PARTICLES_VERTEX_MODELSPACE]);
    
    
        glGenBuffers(1, &ures[RES_SOLID_PARTICLES_CENTER_BUFFER]);
        glBindBuffer(GL_ARRAY_BUFFER,
                     ures[RES_SOLID_PARTICLES_CENTER_BUFFER]);
        glBufferData(GL_ARRAY_BUFFER,
                     MAX_PARTICLES * sizeof(f32) * 4,
                     0,
                     GL_STREAM_DRAW);
        glVertexAttribPointer(res[RES_SOLID_PARTICLES_CENTER],
                              4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(res[RES_SOLID_PARTICLES_CENTER]);
        glVertexAttribDivisor(res[RES_SOLID_PARTICLES_CENTER], 1);

    
        glGenBuffers(1, &ures[RES_SOLID_PARTICLES_SCALING_BUFFER]);
        glBindBuffer(GL_ARRAY_BUFFER,
                     ures[RES_SOLID_PARTICLES_SCALING_BUFFER]);
        glBufferData(GL_ARRAY_BUFFER,
                     MAX_PARTICLES * sizeof(f32) * 2,
                     0,
                     GL_STREAM_DRAW);
        glVertexAttribPointer(res[RES_SOLID_PARTICLES_SCALING],
                              2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(res[RES_SOLID_PARTICLES_SCALING]);
        glVertexAttribDivisor(res[RES_SOLID_PARTICLES_SCALING], 1);

    
        glGenBuffers(1, &ures[RES_SOLID_PARTICLES_DRAW_COLOR_BUFFER]);
        glBindBuffer(GL_ARRAY_BUFFER,
                     ures[RES_SOLID_PARTICLES_DRAW_COLOR_BUFFER]);
        glBufferData(GL_ARRAY_BUFFER,
                     MAX_PARTICLES * sizeof(f32) * 4,
                     0,
                     GL_STREAM_DRAW);
        glVertexAttribPointer(res[RES_SOLID_PARTICLES_DRAW_COLOR],
                              4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(res[RES_SOLID_PARTICLES_DRAW_COLOR]);
        glVertexAttribDivisor(res[RES_SOLID_PARTICLES_DRAW_COLOR], 1);
    
    
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(index_data),
                     index_data,
                     GL_STATIC_DRAW);
    }


    /////////////////////////////////////////////////////////////
    // texture program
    /////////////////////////////////////////////////////////////
    {
        if (!load_program(&ures[RES_TEXTURES_PROG],
                          "shaders/texture_vertex_shader.glsl",
                          "shaders/texture_fragment_shader.glsl")) {
            platform_debug_print("Error loading GL program");
            assert_(false); 
        }

        glGenVertexArrays(1,&ures[RES_TEXTURES_VAO_RECT]);
        glBindVertexArray(ures[RES_TEXTURES_VAO_RECT]);
        
        res[RES_TEXTURES_VERTEX_MODELSPACE] =
            glGetAttribLocation(ures[RES_TEXTURES_PROG], "vertex_modelspace");
        res[RES_TEXTURES_VERTEX_UV] =
            glGetAttribLocation(ures[RES_TEXTURES_PROG], "vertex_uv");
        res[RES_TEXTURES_TRANSFORM] =
            glGetUniformLocation(ures[RES_TEXTURES_PROG], "texture_transform");
        res[RES_TEXTURES_UV_TRANSFORM] =
            glGetUniformLocation(ures[RES_TEXTURES_PROG], "uv_transform");
        res[RES_TEXTURES_SAMPLER] =
            glGetUniformLocation(ures[RES_TEXTURES_PROG], "texture_sampler");
        res[RES_TEXTURES_TINT] =
            glGetUniformLocation(ures[RES_TEXTURES_PROG], "tint");
        res[RES_TEXTURES_LIGHTING] =
            glGetUniformLocation(ures[RES_TEXTURES_PROG], "lighting");
        res[RES_TEXTURES_TEXTURE_ID] =
            glGetUniformLocation(ures[RES_TEXTURES_PROG], "texture_id_in");


        GLfloat vertex_data[] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f
        };

        GLfloat uv_data[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
        };

        u32 index_data[] = { 0, 1, 2, 3 };

        u32 vbo;
        u32 uv_vbo;
        u32 ibo;

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vertex_data),
                     vertex_data,
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(res[RES_TEXTURES_VERTEX_MODELSPACE]);
        glVertexAttribPointer(res[RES_TEXTURES_VERTEX_MODELSPACE],
                              3, GL_FLOAT, GL_FALSE, 0, 0);

        glGenBuffers(1, &uv_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(uv_data),
                     uv_data,
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(res[RES_TEXTURES_VERTEX_UV]);
        glVertexAttribPointer(res[RES_TEXTURES_VERTEX_UV],
                              2, GL_FLOAT, GL_FALSE, 0, 0);

        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(index_data),
                     index_data,
                     GL_STATIC_DRAW);

    }

    /////////////////////////////////////////////////////////////
    // solids program
    /////////////////////////////////////////////////////////////

    gl_program_t solids_program;
    glGenVertexArrays(1,&solids_program.vaos[GL_VAO_RECT]);
    glBindVertexArray(solids_program.vaos[GL_VAO_RECT]);

    if (!load_program(&solids_program.id,
                      "shaders/simple_vertex_shader.glsl",
                      "shaders/simple_fragment_shader.glsl")) {
        platform_debug_print("Error loading GL program");
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

    i32 len = (i32)ARRAY_SIZE(circle_index_data);
    f32 angle = f2PI / ((f32)len - 2); 
    for (int i = 1; i < len; ++i) {
        circle_vertex_data[i*3] = sin(angle * (f32)(i-1));
        circle_vertex_data[i*3 + 1] = cos(angle * (f32)(i-1));
        circle_vertex_data[i*3 + 2] = 0.0f;
        circle_index_data[i] = (u32)i;
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
    solids_program.uniforms[GL_UNIFORM_LIGHTING] =
        glGetUniformLocation(solids_program.id, "lighting");

    result.programs[GL_PROG_SOLIDS] = solids_program;

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

    u32 texture_id_map;
    glGenRenderbuffers(1, &texture_id_map);
    glBindRenderbuffer(GL_RENDERBUFFER, texture_id_map);
    glRenderbufferStorage(GL_RENDERBUFFER,
                          GL_DEPTH_COMPONENT,
                          width,
                          height);
    glBindTexture(GL_TEXTURE_2D, texture_id_map);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_R32I,
                 width,
                 height,
                 0,
                 GL_RED_INTEGER,
                 GL_UNSIGNED_INT,
                 0);
        
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT1,
                           GL_TEXTURE_2D,
                           texture_id_map,
                           0);

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

font_spec_t load_font(char* font_file_name, f32 pixel_height) {
    font_spec_t result;

    tex2 texture;

    platform_read_entire_file_result_t file =
        platform_read_entire_file(font_file_name);

    texture.width = 1024;
    texture.height = 1024;
    texture.pitch = 1024;
    texture.pixels = (u32*)malloc((size_t)texture.width * (size_t)texture.height * sizeof(u32));

    u8* bitmap = (u8*)malloc((size_t)texture.width * (size_t)texture.height * sizeof(u8));

    stbtt_BakeFontBitmap(file.contents,
                         0,
                         pixel_height,
                         bitmap,
                         texture.width,
                         texture.height,
                         32,
                         ARRAY_SIZE(result.baked_chars),
                         result.baked_chars);

    for (i32 i = 0; i < texture.height; ++i) {
        for (i32 j = 0; j < texture.width; ++j) {
            texture.pixels[i * texture.width + j] =
                0xffffff00 | bitmap[(texture.height - i - 1) * texture.width + j];
        }
    }

    platform_free_file_memory(file.contents);
    free(bitmap);

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

    result.texture = texture;
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
    texture.pixels = (u32*)malloc((size_t)width * (size_t)height * sizeof(u32));

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

        result.width = header->width * scaling;
        result.pitch = result.width;
        result.height = header->height * scaling;
        result.pixels = (u32*)malloc((size_t)result.width * (size_t)result.height * sizeof(u32));

        u32 red_mask = header->red_mask;
        u32 blue_mask = header->blue_mask;
        u32 green_mask = header->green_mask;
        u32 alpha_mask = ~(red_mask | green_mask | blue_mask);

        i32 red_shift = find_least_significant_set_bit(red_mask);
        i32 green_shift = find_least_significant_set_bit(green_mask);
        i32 blue_shift = find_least_significant_set_bit(blue_mask);
        i32 alpha_shift = find_least_significant_set_bit(alpha_mask);

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
                             b32 parallax) {
    i32 i = render_group->objects.push_unassigned();
    render_object_t* obj = render_group->objects.at(i);
    obj->type = RENDER_TYPE_CIRCLE;
    obj->render_circle.color = color;
    obj->render_circle.center = center;
    obj->render_circle.radius = radius;
    obj->z = z;
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

render_object_t* push_color_picker(render_group_t* render_group,
                                   v3 hsv,
                                   rect r) {

    i32 i = render_group->objects.push_unassigned();
    render_object_t* obj = render_group->objects.at(i);
    obj->type = RENDER_TYPE_COLOR_PICKER;
    obj->render_color_picker.hsv = hsv;
    obj->render_color_picker.center = 0.5f * (r.max + r.min);
    obj->render_color_picker.diagonal = (r.max - r.min);
    obj->parallax = 0.0f;
    obj->z = 0.0f;
    return obj;
}

render_object_t* push_rect_outline(render_group_t* render_group,
                           color_t color,
                           v2 center,
                           v2 diagonal,
                           f32 orientation,
                           f32 z,
                           b32 parallax) {
    i32 i = render_group->objects.push_unassigned();
    render_object_t* obj = render_group->objects.at(i);
    obj->type = RENDER_TYPE_RECT_OUTLINE;
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
                  f32 z) {
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
    // if (flags & RENDER_WIREFRAME) {
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0);
    // } else {
    //     glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);
    // }
}

void draw_gl_color_picker(gl_programs_t* programs,
                          camera_t camera,
                          render_color_picker_t color_picker,
                          f32 z) {
    TIMED_FUNC();
    // our base rect is simply a square at the origin with sides of length 1.0f,
    // transform that square to get our draw rect

    m4x4 model_rotate = identity_4x4();
    m4x4 model_scale = identity_4x4();
    m4x4 model_translate = identity_4x4();
    //scale
    model_scale.r1.c1 = color_picker.diagonal.x;
    model_scale.r2.c2 = color_picker.diagonal.y;
    // translate
    model_translate.r1.c4 = color_picker.center.x;
    model_translate.r2.c4 = color_picker.center.y;
    model_translate.r3.c4 = z;

    m4x4 model = model_translate * model_rotate * model_scale;
    m4x4 view = get_view_transform_4x4(camera);
    m4x4 transform = view * model;

    glUniformMatrix4fv(programs->i_res_ids[RES_COLOR_PICKER_TRANSFORM],
                       1,
                       GL_TRUE,
                       transform.vals);

    glUniform3f(programs->i_res_ids[RES_COLOR_PICKER_HSV],
                color_picker.hsv.h,
                color_picker.hsv.s,
                color_picker.hsv.v);
    v2 min_p = (color_picker.center - 0.5f * color_picker.diagonal);
    v2 max_p = (color_picker.center + 0.5f * color_picker.diagonal);
    glUniform2f(programs->i_res_ids[RES_COLOR_PICKER_MIN_P], min_p.x, min_p.y);
    glUniform2f(programs->i_res_ids[RES_COLOR_PICKER_MAX_P], max_p.x, max_p.y);

    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);
}

void draw_gl_circle(gl_program_t* program,
                    camera_t camera,
                    render_circle_t circle,
                    f32 z) {
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

    // if (flags & RENDER_WIREFRAME) {
    //     glDrawArrays(GL_LINE_STRIP, 1, vertex_count  - 1);
    // } else {
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
    // }
}

void draw_gl_texture(gl_programs_t* programs,
                     camera_t camera,
                     render_texture_t texture,
                     f32 z,
                     i32 render_item_index) {
    TIMED_FUNC();

    i32* res = programs->i_res_ids;

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
    glUniform1i(res[RES_TEXTURES_SAMPLER], 0);
    glUniform1i(res[RES_TEXTURES_TEXTURE_ID], render_item_index);

    glUniform4f(res[RES_TEXTURES_TINT],
                texture.tint.r,
                texture.tint.g,
                texture.tint.b,
                texture.tint.a);

    glUniformMatrix4fv(res[RES_TEXTURES_TRANSFORM],
                       1,
                       GL_TRUE,
                       transform.vals);
    glUniformMatrix3fv(res[RES_TEXTURES_UV_TRANSFORM],
                       1,
                       GL_TRUE,
                       uv_transform.vals);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);
}

void setup_gl_for_type(gl_programs_t* programs,
                       u32 type) {

    // i32* res = programs->i_res_ids;
    u32* ures = programs->u_res_ids;

    switch (type) {
        case RENDER_TYPE_CIRCLE: {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glUseProgram(programs->programs[GL_PROG_SOLIDS].id);
            glBindVertexArray(programs->programs[GL_PROG_SOLIDS].vaos[GL_VAO_CIRCLE]);
        } break;
        case RENDER_TYPE_TEXTURE: {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glUseProgram(ures[RES_TEXTURES_PROG]);
            glBindVertexArray(ures[RES_TEXTURES_VAO_RECT]);
            glActiveTexture(GL_TEXTURE0);
        } break;
        case RENDER_TYPE_CIRCLE_OUTLINE: {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glUseProgram(programs->programs[GL_PROG_SOLIDS].id);
            glBindVertexArray(programs->programs[GL_PROG_SOLIDS].vaos[GL_VAO_CIRCLE]);
        } break;
        case RENDER_TYPE_COLOR_PICKER: {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glUseProgram(ures[RES_COLOR_PICKER_PROG]);
            glBindVertexArray(ures[RES_COLOR_PICKER_VAO_RECT]);
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

    i32* res = programs->i_res_ids;

    setup_gl_for_type(programs, RENDER_TYPE_TEXTURE);

    glUniform4f(res[RES_TEXTURES_LIGHTING],
                1.0f,
                1.0f,
                1.0f,
                1.0f);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.id);
    glViewport(0,0,dest.width, dest.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, source.texture);
    glUniform1i(res[RES_TEXTURES_SAMPLER], 0);

    m4x4 transform = identity_4x4();
    transform.r1.c1 = 2.0f;
    transform.r2.c2 = 2.0f;
    m3x3 uv_transform = identity_3x3();
    glUniformMatrix4fv(res[RES_TEXTURES_TRANSFORM],
                       1,
                       GL_TRUE,
                       transform.vals);
    glUniformMatrix3fv(res[RES_TEXTURES_UV_TRANSFORM],
                       1,
                       GL_TRUE,
                       uv_transform.vals);

    glUniform4f(res[RES_TEXTURES_TINT],
                0.0f,
                0.0f,
                0.0f,
                0.0f);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);

}

void draw_gradient(gl_programs_t* programs,
                   v2 viewport,
                   v2 start,
                   v2 end,
                   rgba_t start_color,
                   rgba_t end_color) {

    TIMED_FUNC();

    i32* res = programs->i_res_ids;
    u32* ures = programs->u_res_ids;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUseProgram(ures[RES_GRADIENT_PROG]);
    glBindVertexArray(ures[RES_GRADIENT_VAO_RECT]);

    glUniform2f(res[RES_GRADIENT_VIEWPORT], viewport.x, viewport.y);
    glUniform2f(res[RES_GRADIENT_GRADIENT_START], start.x, start.y);
    glUniform2f(res[RES_GRADIENT_GRADIENT_END], end.x, end.y);
    glUniform4f(res[RES_GRADIENT_START_COLOR],
                start_color.r, start_color.g, start_color.b, start_color.a);
    glUniform4f(res[RES_GRADIENT_END_COLOR],
                end_color.r, end_color.g, end_color.b, end_color.a);

    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);
}

void draw_rect_particles(transient_state_t* transient_state,
                                gl_programs_t* programs,
                                camera_t camera,
                                render_group_t* render_group,
                                b32 solid) {
    u32 type = solid ? RENDER_TYPE_RECT : RENDER_TYPE_RECT_OUTLINE;

    f32 max_distance_sq = length_squared(2.0f * camera.to_top_right);

    i32* res = programs->i_res_ids;
    u32* ures = programs->u_res_ids;

    i32 particle_count = 0;
    v4* particle_center_data = transient_state->particle_center_data;
    v2* particle_scaling_data = transient_state->particle_scaling_data;
    rgba_t* particle_color_data = transient_state->particle_color_data;

    glUseProgram(ures[RES_SOLID_PARTICLES_PROG]);
    glUniform4f(res[RES_SOLID_PARTICLES_LIGHTING],
                render_group->lighting.r,
                render_group->lighting.g,
                render_group->lighting.b,
                render_group->lighting.a);

    for (int i = 0; i < render_group->objects.count; ++i) {
        render_object_t* obj = render_group->objects.at(i);

        if (obj->type != type) {
            continue;
        }

        f32 parallax = obj->parallax ? (1.0f - obj->z) : 1.0f;
        if (length_squared(camera.center * parallax - obj->center) > max_distance_sq) {
            continue;
        }

        assert_(particle_count < MAX_PARTICLES);
        
        particle_center_data[particle_count] = 
            v4 {obj->center.x, obj->center.y, obj->z, parallax};
        color_t color = obj->render_rect.color;
        particle_color_data[particle_count] =
            rgba_t {color.r,color.g,color.b,1.0f};
        particle_scaling_data[particle_count] =
            v2 {obj->render_rect.diagonal};

        particle_count++;
    }

    if (solid) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    glUseProgram(ures[RES_SOLID_PARTICLES_PROG]);
    glBindVertexArray(ures[RES_SOLID_PARTICLES_VAO_RECT]);

    glBindBuffer(GL_ARRAY_BUFFER,
                 ures[RES_SOLID_PARTICLES_CENTER_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER,
                 MAX_PARTICLES * (i32)sizeof(*particle_center_data),
                 0,
                 GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    particle_count * (i32)sizeof(*particle_center_data),
                    particle_center_data);

    glBindBuffer(GL_ARRAY_BUFFER,
                 ures[RES_SOLID_PARTICLES_SCALING_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER,
                 MAX_PARTICLES * (i32)sizeof(*particle_scaling_data),
                 0,
                 GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    particle_count * (i32)sizeof(*particle_scaling_data),
                    particle_scaling_data);

    glBindBuffer(GL_ARRAY_BUFFER,
                 ures[RES_SOLID_PARTICLES_DRAW_COLOR_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER,
                 MAX_PARTICLES * (i32)sizeof(*particle_color_data),
                 0,
                 GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    particle_count * (i32)sizeof(*particle_color_data),
                    particle_color_data);

    m4x4 view_transform = get_view_transform_4x4(camera);

    glUniformMatrix4fv(res[RES_SOLID_PARTICLES_VIEW_TRANSFORM],
                       1,
                       GL_TRUE,
                       view_transform.vals);

    if (solid) {
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, particle_count);
    } else {
        glDrawArraysInstanced(GL_LINE_LOOP, 0, 4, particle_count);
    }
}

void
draw_solid_rect_particles(transient_state_t* transient_state,
                          gl_programs_t* programs,
                          camera_t camera,
                          render_group_t* render_group) {
    draw_rect_particles(transient_state,
                        programs,
                        camera,
                        render_group,
                        true);
}

void
draw_outlined_rect_particles(transient_state_t* transient_state,
                          gl_programs_t* programs,
                          camera_t camera,
                          render_group_t* render_group) {
    draw_rect_particles(transient_state,
                        programs,
                        camera,
                        render_group,
                        false);
}

void draw_render_group(transient_state_t* transient_state,
                       gl_programs_t* programs,
                       camera_t camera,
                       render_group_t* render_group) {
    TIMED_FUNC();

    const GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(ARRAY_SIZE(buffers), buffers);
    
    i32* res = programs->i_res_ids;
    u32* ures = programs->u_res_ids;

    f32 max_distance_sq = length_squared(2.0f * camera.to_top_right);

    u32 types[] = {
        RENDER_TYPE_CIRCLE,
        RENDER_TYPE_TEXTURE,
        RENDER_TYPE_CIRCLE_OUTLINE,
		RENDER_TYPE_COLOR_PICKER
    };

    glUseProgram(ures[RES_TEXTURES_PROG]);
    glUniform4f(res[RES_TEXTURES_LIGHTING],
                render_group->lighting.r,
                render_group->lighting.g,
                render_group->lighting.b,
                render_group->lighting.a);

    i32 len = (i32)ARRAY_SIZE(types);
    for (int i = 0; i < len; ++i) {
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
                case RENDER_TYPE_CIRCLE_OUTLINE:
                case RENDER_TYPE_CIRCLE: {
                    draw_gl_circle(&programs->programs[GL_PROG_SOLIDS],
                                   camera,
                                   obj->render_circle,
                                   obj->z);
                } break;
                case RENDER_TYPE_TEXTURE: {
                    draw_gl_texture(programs,
                                    camera,
                                    obj->render_texture,
                                    obj->z,
                                    j);
                } break;
                case RENDER_TYPE_COLOR_PICKER: {
                    draw_gl_color_picker(programs,
                                         camera,
                                         obj->render_color_picker,
                                         obj->z);
                } break;
            }
        }
    }

    draw_solid_rect_particles(transient_state, programs, camera, render_group);
    draw_outlined_rect_particles(transient_state, programs, camera, render_group);
}

void clear_render_group(render_group_t* render_group) {
    render_group->objects.count = 0;
}

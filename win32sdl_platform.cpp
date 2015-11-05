//
// Created by doug on 4/16/15.
//

#include <math.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <intrin.h>

// #include <unistd.h>
#include <locale.h>
#include <Windows.h>

#include "SDL2/SDL.h"

#include "game.h"
#include "win32sdl_platform.h"


void platform_start_task(task_queue_t* queue, task_callback_t* callback, void* data) {
    i32 next_write_index = (queue->write_index + 1) % TASK_QUEUE_MAX_ENTRIES;
    assert_(next_write_index != queue->read_index);

    task_t* task = queue->tasks + queue->write_index;
    task->callback = callback;
    task->data = data;
    queue->write_index = next_write_index;
    InterlockedIncrement((LONG volatile*)&queue->remaining);
    SDL_SemPost(queue->semaphore);
}

b32 platform_execute_next_task(task_queue_t* queue) {
    b32 sleep = false;

    i32 original = queue->read_index;
    i32 read_index = (original + 1) % TASK_QUEUE_MAX_ENTRIES;
    if (original != queue->write_index) {
        if (InterlockedCompareExchange((LONG volatile*)&queue->read_index,
                                        read_index,
                                        original) == original) {
            task_t* task = queue->tasks + original;
            task->callback(queue, task->data);
            InterlockedDecrement((LONG volatile*)&queue->remaining);
        }
    } else {
        sleep = true;
    }

    return sleep;
}

void platform_wait_on_queue(task_queue_t* queue) {
    while (queue->remaining) {
        platform_execute_next_task(queue);
    }
}

int thread_func(void* ptr) {
    task_queue_t* queue = (task_queue_t*)ptr;

    while (true) {
        if (platform_execute_next_task(queue)) {
            SDL_SemWait(queue->semaphore);
        }
    }

    return 0;
}

void platform_free_file_memory(void* memory) {
    free(memory);
}

platform_read_entire_file_result_t platform_read_entire_file(const char * filename) {
    platform_read_entire_file_result_t result = {};

    FILE *f = fopen(filename, "rb");

    if (f == 0) {
        printf("Failed to open file %s\n", filename);
        return result;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = (char *)malloc(fsize + 1);
    if (!fread(string, fsize, 1, f)) {
        printf("No results from fread\n");
    }
    fclose(f);

    string[fsize] = 0;

    result.contents = (u8 *)string;
    result.content_size = (u32) fsize;

    return result;
}

b32 handle_sdl_event(SDL_Event* event, platform_context_t* context) {
    b32 should_quit = false;
    switch (event->type) {
        case SDL_QUIT: {
            printf("SDL_QUIT\n");
            should_quit = true;
        } break;
        case SDL_WINDOWEVENT: {
            switch (event->window.event) {
                case SDL_WINDOWEVENT_RESIZED: {
                    printf("SDL_WINDOWEVENT_RESIZED (%d, %d)\n", event->window.data1,
                           event->window.data2);
                } break;
                case SDL_WINDOWEVENT_EXPOSED: {
                    printf("SDL_WINDOWEVENT_EXPOSED\n");
                } break;
            } break;
        } break;
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            SDL_Keycode key_code = event->key.keysym.sym;
            b32 ended_down = event->key.state != SDL_RELEASED;

            if (event->type == SDL_KEYUP || event->key.repeat == 0) {
                switch (key_code) {
                    case SDLK_UP: {
                        context->next_input->button_a.ended_down = ended_down;
                        context->next_input->button_a.transition_count++;
                    } break;
                    case SDLK_DOWN: {
                        context->next_input->button_x.ended_down = ended_down;
                        context->next_input->button_x.transition_count++;
                    } break;
                    case SDLK_LEFT: {
                        context->next_input->joystick_l.position.x = ended_down ? -1.0f : 0.0f;
                    } break;
                    case SDLK_RIGHT: {
                        context->next_input->joystick_l.position.x = ended_down ? 1.0f : 0.0f;
                    } break;
                }
            }
        } break;
        case SDL_CONTROLLERDEVICEADDED: {
            if (context->controller_handle) {
                SDL_GameControllerClose(context->controller_handle);
            }
            context->controller_handle = SDL_GameControllerOpen(event->cdevice.which);
        } break;
        case SDL_CONTROLLERDEVICEREMOVED: {
            if (context->controller_handle) {
                SDL_GameControllerClose(context->controller_handle);
            }
        } break;
    }

    SDL_GameController* handle = context->controller_handle;
    if(handle && SDL_GameControllerGetAttached(handle))
    {
        // NOTE: We have a controller with index ControllerIndex.
        b32 up = SDL_GameControllerGetButton(handle, SDL_CONTROLLER_BUTTON_DPAD_UP);
        b32 down = SDL_GameControllerGetButton(handle, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        b32 left = SDL_GameControllerGetButton(handle, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        b32 right = SDL_GameControllerGetButton(handle, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        b32 start = SDL_GameControllerGetButton(handle, SDL_CONTROLLER_BUTTON_START);
        b32 back = SDL_GameControllerGetButton(handle, SDL_CONTROLLER_BUTTON_BACK);
        b32 l_shoulder = SDL_GameControllerGetButton(handle, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        b32 r_shoulder = SDL_GameControllerGetButton(handle, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        b32 button_a = SDL_GameControllerGetButton(handle, SDL_CONTROLLER_BUTTON_A);
        b32 button_b = SDL_GameControllerGetButton(handle, SDL_CONTROLLER_BUTTON_B);
        b32 button_x = SDL_GameControllerGetButton(handle, SDL_CONTROLLER_BUTTON_X);
        b32 button_y = SDL_GameControllerGetButton(handle, SDL_CONTROLLER_BUTTON_Y);

        i16 stick_x = SDL_GameControllerGetAxis(handle, SDL_CONTROLLER_AXIS_LEFTX);
        i16 stick_y = SDL_GameControllerGetAxis(handle, SDL_CONTROLLER_AXIS_LEFTY);

        context->next_input->up.ended_down = up;
        context->next_input->down.ended_down = down;
        context->next_input->left.ended_down = left;
        context->next_input->right.ended_down = right;

        context->next_input->button_a.ended_down = button_a;
        context->next_input->button_b.ended_down = button_b;
        context->next_input->button_x.ended_down = button_x;
        context->next_input->button_y.ended_down = button_y;

        context->next_input->joystick_l.position.x = ((f32)stick_x) / 32768.0f;        
        context->next_input->joystick_l.position.y = ((f32)stick_y) / 32768.0f;        
    }

    return should_quit;
}

SDL_GameController* find_controller_handle() {
    i32 num_joysticks = SDL_NumJoysticks();

    for (int i = 0; i < num_joysticks; ++i) {
        printf("%s\n", SDL_GameControllerName(SDL_GameControllerOpen(i)));
        return SDL_GameControllerOpen(i);
    }

    printf("No controller found. Joysticks attached: %d\n", num_joysticks);
    return 0;
}

f32 get_seconds_elapsed(u64 old, u64 current) {
    return ((f32)(current - old) / (f32)(SDL_GetPerformanceFrequency()));
}

void printer_task(task_queue_t* queue, void* data) {
    char* as_str = (char*)data;
    printf("%s\n", as_str);
}

 
void check_sdl_error(int line = -1) {
    const char *error = SDL_GetError();
    if (*error != '\0')
    {
        char buffer[1024];
        sprintf(buffer, "SDL Error: %s\n", error);
        if (line != -1)
            sprintf(buffer, " + line: %i\n", line);
        SDL_ClearError();
        OutputDebugString(buffer);
    }
}

GLint check_shader_status(GLuint item_id, GLenum status_id) {
    GLint result = GL_FALSE;
    GLint info_log_length = 0;
    glGetShaderiv(item_id, status_id, &result);
    if (result != GL_TRUE) {
        glGetShaderiv(item_id, GL_INFO_LOG_LENGTH, &info_log_length);
        char* buffer = (char*)alloca(info_log_length);
        glGetShaderInfoLog(item_id, info_log_length, NULL, buffer);
        OutputDebugString(buffer);
    }

    return result;
}

GLint check_program_status(GLuint item_id, GLenum status_id) {
    GLint result = GL_FALSE;
    GLint info_log_length = 0;
    glGetProgramiv(item_id, status_id, &result);
    if (result != GL_TRUE) {
        glGetProgramiv(item_id, GL_INFO_LOG_LENGTH, &info_log_length);
        char* buffer = (char*)alloca(info_log_length);
        glGetProgramInfoLog(item_id, info_log_length, NULL, buffer);
        OutputDebugString(buffer);
    }

    return result;
}

b32 load_program(GLuint* program_result,
                 const char* vertex_path,
                 const char* fragment_path) {
    platform_read_entire_file_result_t vertex_file = platform_read_entire_file(vertex_path);
	char* vertex_code = (char*)vertex_file.contents;
    char* fragment_code = (char*)platform_read_entire_file(fragment_path).contents;

    GLuint program_id = glCreateProgram();
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

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

const char* to_print = "hello, world";

int CALLBACK WinMain(
  _In_ HINSTANCE hInstance,
  _In_ HINSTANCE hPrevInstance,
  _In_ LPSTR     lpCmdLine,
  _In_ int       nCmdShow
) {
    setlocale(LC_NUMERIC, "");

    platform_context_t context = {0};

    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        printf("Unable to init SDL: %s\n", SDL_GetError());
        return 1;
    }

    task_queue_t primary_queue = {0};
    primary_queue.semaphore = SDL_CreateSemaphore(0);
    task_queue_t secondary_queue = {0};
    secondary_queue.semaphore = SDL_CreateSemaphore(0);
    task_queue_t render_queue = {0};
    render_queue.semaphore = SDL_CreateSemaphore(0);

    platform_services_t platform = {0};
    platform.primary_queue = &primary_queue;
    platform.secondary_queue = &secondary_queue;
    platform.render_queue = &render_queue;
    platform.start_task = &platform_start_task;
    platform.wait_on_queue = &platform_wait_on_queue;

    for (int i = 0; i < 8; ++i) {
        char buffer[10];
        sprintf(buffer, "thread%d", i);
        SDL_CreateThread(thread_func, buffer, (void*)&render_queue);
    }

    check_sdl_error(__LINE__);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    context.window = SDL_CreateWindow("hello",
                                      SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED,
                                      START_WIDTH,
                                      START_HEIGHT,
                                      SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    check_sdl_error(__LINE__);

    context.gl_context = SDL_GL_CreateContext(context.window);
    check_sdl_error(__LINE__);

    glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
        OutputDebugString("Error initializing GLEW");
    }

    if( SDL_GL_SetSwapInterval(1) < 0 ){
        check_sdl_error(__LINE__);
    }
    
    // glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint rect_program;
    if (!load_program(&rect_program,
                      "shaders/rect_vertex_shader.glsl",
                      "shaders/rect_fragment_shader.glsl")) {
        OutputDebugString("Error loading GL program");
        return -1; 
    }

    GLfloat rect_vertex_data[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };
    GLuint rect_index_data[] = { 0, 1, 2, 3 };

    GLuint rect_vbo;
    GLuint rect_ibo;

    glGenBuffers(1, &rect_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(rect_vertex_data),
                 rect_vertex_data,
                 GL_STATIC_DRAW);

    glGenBuffers(1, &rect_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rect_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(rect_index_data),
                 rect_index_data,
                 GL_STATIC_DRAW);

    GLint rect_modelspace = glGetAttribLocation(rect_program, "vertex_modelspace");
    GLint rect_transform = glGetUniformLocation(rect_program, "rect_transform");
    GLint rect_color = glGetUniformLocation(rect_program, "rect_color");

    platform.rect_program.id = rect_program;
    platform.rect_program.transform_loc = rect_transform;
    platform.rect_program.color_loc = rect_color;
    platform.rect_program.vertex_modelspace_loc = rect_modelspace;
    platform.rect_program.vbo = rect_vbo;
    platform.rect_program.ibo = rect_ibo;

    GLuint texture_program;
    if (!load_program(&texture_program,
                      "shaders/texture_vertex_shader.glsl",
                      "shaders/texture_fragment_shader.glsl")) {
        OutputDebugString("Error loading GL program");
        return -1; 
    }

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

    GLuint texture_index_data[] = { 0, 1, 2, 3 };

    GLuint texture_vbo;
    GLuint texture_uv_vbo;
    GLuint texture_ibo;

    glGenBuffers(1, &texture_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, texture_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(texture_vertex_data),
                 texture_vertex_data,
                 GL_STATIC_DRAW);

    glGenBuffers(1, &texture_uv_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, texture_uv_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(texture_uv_data),
                 texture_uv_data,
                 GL_STATIC_DRAW);

    glGenBuffers(1, &texture_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texture_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(texture_index_data),
                 texture_index_data,
                 GL_STATIC_DRAW);

    platform.texture_program.id = texture_program;
    platform.texture_program.vbo = texture_vbo;
    platform.texture_program.uv_vbo = texture_uv_vbo;
    platform.texture_program.ibo = texture_ibo;
    platform.texture_program.transform_loc =
        glGetUniformLocation(texture_program, "texture_transform");
    platform.texture_program.uv_transform_loc =
        glGetUniformLocation(texture_program, "uv_transform");
    platform.texture_program.vertex_modelspace_loc = 
        glGetAttribLocation(texture_program, "vertex_modelspace");
    platform.texture_program.vertex_uv_loc = 
        glGetAttribLocation(texture_program, "vertex_uv");
    platform.texture_program.texture_sampler_loc =
        glGetUniformLocation(texture_program, "texture_sampler");

    glClearColor(0.784f, 0.8745f, 0.925f, 1.0f);

    context.renderer = SDL_CreateRenderer(context.window, -1, SDL_RENDERER_PRESENTVSYNC);
    check_sdl_error(__LINE__);

    SDL_RendererInfo renderer_info;
    SDL_GetRendererInfo(context.renderer, &renderer_info);

    if ((renderer_info.flags & SDL_RENDERER_ACCELERATED) == 0 || 
        (renderer_info.flags & SDL_RENDERER_TARGETTEXTURE) == 0) {
        assert_(false);
    }

    if (!context.renderer) {
        printf("Unable to create renderer: %s\n", SDL_GetError());
        return 1;
    }

    context.texture = SDL_CreateTexture(context.renderer,
                                        SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        START_WIDTH,
                                        START_HEIGHT);

    if (!context.texture) {
        printf("Unable to create texture: %s\n", SDL_GetError());
        return 1;
    }

    size_t pixel_bytes = START_WIDTH * START_HEIGHT * 4;
    void* pixels = malloc(pixel_bytes);
    // void* pixels = aligned_alloc(64, pixel_bytes + (pixel_bytes % 64));

    const u64 one_gig = 1024LL * 1024LL * 1024LL;
    void* game_memory = calloc(one_gig, sizeof(u8));

    context.controller_handle = find_controller_handle();

    u32 frame_index = 0;
    u32 s = 0;
    bool running = true;

    game_input_t prev_input = {};
    game_input_t next_input = {};
    context.next_input = &next_input;
    context.prev_input = &prev_input;

    u64 last_counter = SDL_GetPerformanceCounter();
    const f32 target_seconds_per_frame = 1.0f / (f32)FRAME_RATE;
    while(running) {
        f32 elapsed = get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter());
        if (elapsed < target_seconds_per_frame) {
            u32 sleep_time = ((target_seconds_per_frame - elapsed) * 1000) - 1;
            SDL_Delay(sleep_time);
            elapsed = get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter());
            while (get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter()) <
                   target_seconds_per_frame) { }
        }

        TIMED_BLOCK(main_run_loop);

        f32 dt = get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter());
        // {
        //     char char_buffer[256];
        //     sprintf(char_buffer, "\nfps: %d\n", (i32)(1.0f / dt));
        //     OutputDebugStringA(char_buffer);
        // }

        last_counter = SDL_GetPerformanceCounter();

        ZERO_STRUCT(next_input);
        next_input.up.ended_down = prev_input.up.ended_down;
        next_input.down.ended_down = prev_input.down.ended_down;
        next_input.left.ended_down = prev_input.left.ended_down;
        next_input.right.ended_down = prev_input.right.ended_down;
        next_input.lshift.ended_down = prev_input.lshift.ended_down;
        next_input.rshift.ended_down = prev_input.rshift.ended_down;

        next_input.button_a.ended_down = prev_input.button_a.ended_down;
        next_input.button_b.ended_down = prev_input.button_b.ended_down;
        next_input.button_x.ended_down = prev_input.button_x.ended_down;
        next_input.button_y.ended_down = prev_input.button_y.ended_down;
        next_input.button_l_bumper.ended_down =
            prev_input.button_l_bumper.ended_down;
        next_input.button_r_bumper.ended_down =
            prev_input.button_r_bumper.ended_down;
        next_input.button_l_stick.ended_down =
            prev_input.button_l_stick.ended_down;
        next_input.button_r_stick.ended_down =
            prev_input.button_r_stick.ended_down;

        next_input.analog_l_trigger.value = prev_input.analog_l_trigger.value;
        next_input.analog_r_trigger.value = prev_input.analog_r_trigger.value;

        next_input.joystick_l.position = prev_input.joystick_l.position;
        next_input.joystick_r.position = prev_input.joystick_r.position;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (handle_sdl_event(&event, &context)) {
                running = false;
            }
        }

        next_input.analog_l_trigger.delta =
                next_input.analog_l_trigger.value -
                prev_input.analog_l_trigger.value;
        next_input.analog_r_trigger.delta =
                next_input.analog_r_trigger.value -
                prev_input.analog_r_trigger.value;

        next_input.joystick_l.delta =
                next_input.joystick_l.position - prev_input.joystick_l.position;
        next_input.joystick_r.delta =
                next_input.joystick_r.position - prev_input.joystick_r.position;

        video_buffer_description_t game_buffer = {};
        game_buffer.memory = pixels;
        game_buffer.width = START_WIDTH;
        game_buffer.height = START_HEIGHT;
        game_buffer.pitch = START_WIDTH * 4;
        game_buffer.bytes_per_pixel = 4;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        game_update_and_render(platform,
                               (game_state_t*)game_memory,
                               target_seconds_per_frame,
                               game_buffer,
                               next_input);

        prev_input = next_input;

        // if (SDL_UpdateTexture(context.texture,0, pixels, START_WIDTH * 4)) {
        //     printf("Unable to update SDL texture: %s\n", SDL_GetError());
        //     return 1;
        // }

        // SDL_RenderCopy(context.renderer, context.texture, 0, 0);

        // SDL_RenderPresent(context.renderer);

        SDL_GL_SwapWindow(context.window);

        print_debug_log();
    }

    return 0;
}
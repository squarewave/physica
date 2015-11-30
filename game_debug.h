#ifndef PHYSICA_GAME_DEBUG_H
#define PHYSICA_GAME_DEBUG_H
#include <intrin.h>
#include <string.h>
#include "game_render.h"
#include "physica_math.h"

const rgba_t RGBA_GREEN = rgba_t {0.2f, 0.7f, 0.2f, 1.0f};
const rgba_t RGBA_RED = rgba_t {0.7f, 0.2f, 0.2f, 1.0f};
const rgba_t RGBA_ORANGE = rgba_t {0.6f, 0.5f, 0.2f, 1.0f};
const rgba_t RGBA_WHITE = rgba_t {1.0f, 1.0f, 1.0f, 1.0f};
const rgba_t RGBA_BLACK = rgba_t {0.0f, 0.0f, 0.0f, 1.0f};
const rgba_t RGBA_BLUE = rgba_t {0.2f, 0.2f, 0.7f, 1.0f};
const color_t COLOR_GREEN = color_t {0.2f, 0.7f, 0.2f};
const color_t COLOR_RED = color_t {0.7f, 0.2f, 0.2f};
const color_t COLOR_ORANGE = color_t {0.6f, 0.5f, 0.2f};
const color_t COLOR_WHITE = color_t {1.0f, 1.0f, 1.0f};
const color_t COLOR_BLACK = color_t {0.0f, 0.0f, 0.0f};
const color_t COLOR_BLUE = color_t {0.2f, 0.2f, 0.7f};

__inline__ u64 rdtsc() {
    return __rdtsc();
    // u32 a;
    // u32 d;
    // asm volatile
    //     (".byte 0x0f, 0x31 #rdtsc\n" // edx:eax
    //      :"=a"(a), "=d"(d)::);
    // return (((u64) d) << 32) | (u64) a;
}


#define TIMED_BLOCK(ID) timed_block_t timed_block##ID((char*)#ID, __COUNTER__)
#define TIMED_FUNC() timed_block_t timed_func((char*)__FUNCTION__, __COUNTER__)

struct game_state_t;
struct tools_state_t;
struct game_input_t;
struct phy_body_t;

struct debug_block_t {
    char* id;
    u32 call_count;
    u64 total_cycles;
};

struct debug_state_t {
    font_spec_t monospace_font;
    char performance_log[1 << 12];
    phy_body_t* selected;

    b32 draw_wireframes;
    b32 draw_aabb_tree;
    b32 show_performance;
};


const i32 max_debug_blocks = 200;
global i32 max_debug_counter = 0;
global debug_block_t debug_blocks[max_debug_blocks] = {0};

struct timed_block_t {
    i32 block_index;
    u64 start;
    timed_block_t(char* id_str, i32 counter) {
        if (counter > max_debug_counter) {
            max_debug_counter = counter;
        }
        block_index = counter;
        debug_blocks[counter].id = id_str;
        start = rdtsc();
    }

    ~timed_block_t() {
        debug_blocks[block_index].call_count++;
        debug_blocks[block_index].total_cycles += rdtsc() - start;
    }
};

void process_debug_log(tools_state_t* tools_state);

void debug_push_ui_text_f(game_state_t* game_state,
                          tools_state_t* tools_state,
                          window_description_t window,
                          v2 bottom_left,
                          rgba_t color,
                          char* format,
                          ...);

void debug_init(tools_state_t* tools_state);

void debug_update_and_render(game_state_t* game_state,
                             tools_state_t* tools_state,
                             f32 dt,
                             window_description_t window,
                             game_input_t* game_input);

void debug_load_monospace_font(tools_state_t* tools_state);

void debug_push_ui_text(game_state_t* game_state,
                        tools_state_t* tools_state,
                        window_description_t window,
                        v2 top_left,
                        char* text);

void debug_draw_physics(game_state_t* game_state);

#endif
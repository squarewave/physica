#ifndef PHYSICA_GAME_DEBUG_H
#define PHYSICA_GAME_DEBUG_H

#include <string.h>
#include "game_intrinsics.h"
#include "renderer.h"
#include "physica_math.h"

const rgba_ RGBA_GREEN = rgba_ {0.2f, 0.7f, 0.2f, 1.0f};
const rgba_ RGBA_RED = rgba_ {0.7f, 0.2f, 0.2f, 1.0f};
const rgba_ RGBA_ORANGE = rgba_ {0.6f, 0.5f, 0.2f, 1.0f};
const rgba_ RGBA_WHITE = rgba_ {1.0f, 1.0f, 1.0f, 1.0f};
const rgba_ RGBA_BLACK = rgba_ {0.0f, 0.0f, 0.0f, 1.0f};
const rgba_ RGBA_BLUE = rgba_ {0.2f, 0.2f, 0.7f, 1.0f};
const color_ COLOR_GREEN = color_ {0.2f, 0.7f, 0.2f};
const color_ COLOR_RED = color_ {0.7f, 0.2f, 0.2f};
const color_ COLOR_ORANGE = color_ {0.6f, 0.5f, 0.2f};
const color_ COLOR_WHITE = color_ {1.0f, 1.0f, 1.0f};
const color_ COLOR_BLACK = color_ {0.0f, 0.0f, 0.0f};
const color_ COLOR_BLUE = color_ {0.2f, 0.2f, 0.7f};

__inline__ u64 rdtsc() {
    return __rdtsc();
    // u32 a;
    // u32 d;
    // asm volatile
    //     (".byte 0x0f, 0x31 #rdtsc\n" // edx:eax
    //      :"=a"(a), "=d"(d)::);
    // return (((u64) d) << 32) | (u64) a;
}


#define TIMED_BLOCK(ID) timed_block_ timed_block##ID((char*)#ID, __COUNTER__)
#define TIMED_FUNC() timed_block_ timed_func((char*)__FUNCTION__, __COUNTER__)

struct game_state_;
struct tools_state_;
struct game_input_;
struct phy_body_;

struct debug_block_ {
    char* id;
    u32 call_count;
    u64 total_cycles;
};

struct debug_state_ {
    font_spec_ monospace_font;
    char performance_log[1 << 12];
    phy_body_* selected;

    b32 draw_wireframes;
    b32 draw_aabb_tree;
    b32 show_performance;
};


const i32 max_debug_blocks = 200;
global i32 max_debug_counter = 0;
global debug_block_ debug_blocks[max_debug_blocks] = {0};

struct timed_block_ {
    i32 block_index;
    u64 start;
    timed_block_(char* id_str, i32 counter) {
        if (counter > max_debug_counter) {
            max_debug_counter = counter;
        }
        block_index = counter;
        debug_blocks[counter].id = id_str;
        start = rdtsc();
    }

    ~timed_block_() {
        debug_blocks[block_index].call_count++;
        debug_blocks[block_index].total_cycles += rdtsc() - start;
    }
};

void process_debug_log(tools_state_* tools_state);

void debug_push_ui_text_f(game_state_* game_state,
                          tools_state_* tools_state,
                          window_description_ window,
                          v2 bottom_left,
                          rgba_ color,
                          char* format,
                          ...);

void debug_init(tools_state_* tools_state);

void debug_update_and_render(game_state_* game_state,
                             tools_state_* tools_state,
                             f32 dt,
                             window_description_ window,
                             game_input_* game_input);

void debug_load_monospace_font(tools_state_* tools_state);

void debug_push_ui_text(game_state_* game_state,
                        tools_state_* tools_state,
                        window_description_ window,
                        v2 top_left,
                        char* text);

void debug_draw_physics(game_state_* game_state);

void debug_push_ui_text_f(game_state_* game_state,
                          tools_state_* tools_state,
                          window_description_ window,
                          v2 bottom_left,
                          rgba_ color,
                          char* format,
                          ...)
#ifndef _WIN32
__attribute__ ((format (printf, 6, 7)))
#endif
;

#endif

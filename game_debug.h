#ifndef PHYSICA_GAME_DEBUG_H
#define PHYSICA_GAME_DEBUG_H
#include <intrin.h>
#include <string.h>
#include "game_render.h"
#include "physica_math.h"

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
struct game_input_t;
struct phy_body_t;

struct debug_block_t {
    char* id;
    u32 call_count;
    u64 total_cycles;
};

struct debug_state_t {
    glyph_spec_t monospace_font[96];
    phy_body_t* selected;
};


const i32 max_debug_blocks = 200;
global debug_block_t debug_blocks[max_debug_blocks] = {0};

struct timed_block_t {
    i32 block_index;
    u64 start;
    timed_block_t(char* id_str, i32 counter) {
        block_index = counter;
        debug_blocks[counter].id = id_str;
        start = rdtsc();
    }

    ~timed_block_t() {
        debug_blocks[block_index].call_count++;
        debug_blocks[block_index].total_cycles += rdtsc() - start;
    }
};

void print_debug_log() {
    char buffer[256];

    OutputDebugStringA("\n\n");
    for (int i = 0; i < max_debug_blocks; ++i) {
        if (debug_blocks[i].id) {
            sprintf(buffer, "%-32s %14lld cy,    %5d calls\n",
                   debug_blocks[i].id,
                   debug_blocks[i].total_cycles,
                   debug_blocks[i].call_count);
    		OutputDebugStringA(buffer);
            debug_blocks[i].id = 0;
            debug_blocks[i].total_cycles = 0;
            debug_blocks[i].call_count = 0;
        }
	}
}

void debug_init(game_state_t* game_state);

void debug_update_and_render(game_state_t* game_state, f32 dt, game_input_t* game_input);

void debug_load_monospace_font(game_state_t* game_state);

void debug_push_ui_text(game_state_t* game_state, v2 top_left, char* text);

void debug_draw_physics(game_state_t* game_state);

#endif
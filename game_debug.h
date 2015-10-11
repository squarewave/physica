#ifndef PHYSICA_GAME_DEBUG_H
#define PHYSICA_GAME_DEBUG_H
#include <intrin.h>
#include <string.h>

__inline__ u64 rdtsc() {
    return __rdtsc();
    // u32 a;
    // u32 d;
    // asm volatile
    //     (".byte 0x0f, 0x31 #rdtsc\n" // edx:eax
    //      :"=a"(a), "=d"(d)::);
    // return (((u64) d) << 32) | (u64) a;
}

#define TIMED_BLOCK(ID) timed_block_t timed_block##ID((char*)#ID)
#define TIMED_FUNC() timed_block_t timed_func((char*)__FUNCTION__)

struct debug_block_t {
    char* id;
    u32 call_count;
    u64 total_cycles;
};

static debug_block_t debug_blocks[200];
static i32 debug_block_count;
static i64 time_spent_logging = 0;

struct timed_block_t {
    char* id;
    u64 start;
    timed_block_t(char* id_str) {
        id = id_str;
        start = rdtsc();
    }

    ~timed_block_t() {
        u64 d_start = rdtsc();
        b32 found = false;
        for (int i = 0; i < debug_block_count; ++i) {
            if (debug_blocks[i].id == id || !strcmp(debug_blocks[i].id, id)) {
                debug_blocks[i].total_cycles += rdtsc() - start;
                debug_blocks[i].call_count++;
                found = true;
            }
        }

        if (!found) {
            debug_blocks[debug_block_count++] = debug_block_t{id, 1, rdtsc() - start};
        }
        time_spent_logging += rdtsc() - d_start;
    }
};

void print_debug_log() {
    char buffer[256];
    sprintf(buffer, "\ndebug log: %lld\n", time_spent_logging);
    OutputDebugStringA(buffer);
    for (int i = 0; i < debug_block_count; ++i) {
        sprintf(buffer, "%-32s %14lld cy,    %5d calls\n",
               debug_blocks[i].id,
               debug_blocks[i].total_cycles,
               debug_blocks[i].call_count);
		OutputDebugStringA(buffer);
	}

    debug_block_count = 0;
    time_spent_logging = 0;
}

#endif
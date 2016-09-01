//
// Created by doug on 4/16/15.
//

#ifndef GAME_INTRINSICS_H_
#define GAME_INTRINSICS_H_

#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

inline i32 find_least_significant_set_bit(u32 value) {
    for (int i = 0; i < 32; ++i) {
        if (value & (1 << i)) {
            return i;
        }
    }
    return -1;
}

#endif //GAME_INTRINSICS_H_

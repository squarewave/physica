//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_INTRINSICS_H
#define PHYSICA_INTRINSICS_H

i32 find_least_significant_set_bit(u32 value) {
    for (int i = 0; i < 32; ++i) {
        if (value & (1 << i)) {
            return i;
        }
    }
    return -1;
}

#endif //PHYSICA_INTRINSICS_H

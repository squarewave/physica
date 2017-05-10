//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_TYPEDEFS_H
#define PHYSICA_TYPEDEFS_H

#include <assert.h>
#include <stdint.h>

// #ifdef assert
// #undef assert
// #endif
#define assert_(exp) assert(exp)

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef uint32_t b32;
typedef float f32;
typedef double f64;

#define global static
#define persist static

#endif //PHYSICA_TYPEDEFS_H

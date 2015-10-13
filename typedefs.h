//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_TYPEDEFS_H
#define PHYSICA_TYPEDEFS_H

#include <stdint.h>

#ifdef assert
#undef assert
#endif
#define assert(exp) exp || (*((i32*)0) = 0)

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

template <class T>
struct vec {
  T* values;
  i32 count;
  i32 capacity;

  T& operator[] (i32 index) { return this->values[index]; }
  T* at(i32 index) { return this->values + index; }
  i32 push(T val) {
    i32 result = this->count++;
    assert(this->count <= this->capacity);
    this->values[result] = val;
    return result;
  }
  T pop() {
    assert(this->count);
    T result = this->values[--this->count];
    return result;
  }
  i32 push_unassigned() {
    i32 result = this->count++;
    assert(this->count <= this->capacity);
    return result;
  }
};


template <class T>
struct array {
  T *values;
  i32 count;

  T operator[](i32 index) { return this->values[index]; }
  T *at(i32 index) { return this->values + index; }
};

#endif //PHYSICA_TYPEDEFS_H

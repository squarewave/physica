//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_TYPEDEFS_H
#define PHYSICA_TYPEDEFS_H

#include <stdint.h>

#ifdef assert
#undef assert
#endif
#define assert_(exp) exp || (*((i32*)0) = 0)

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

struct memory_arena_t {
    u32 size, used;
    u8* base;
};

void* _push_size(memory_arena_t* arena, u32 size) {
    assert_(arena->used + size <= arena->size);
    u8* result = arena->base + arena->used;
    arena->used += size;
    return result;
};

const i32 SMALL_STACK_SIZE = 256;
const i32 MEDIUM_STACK_SIZE = 1024;
const i32 LARGE_STACK_SIZE = 1024 * 32;

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define PUSH_STRUCT(arena, type) (type *)_push_size(arena, sizeof(type))
#define PUSH_ARRAY(arena, count, type) (type *)_push_size(arena, count * sizeof(type))
#define ZERO_STRUCT(instance) ZeroMemory(&(instance), sizeof(instance))
#define ZERO_ARRAY(instance, count) ZeroMemory(instance, (count) * sizeof(*instance))

template <class T>
struct vec {
  T* values;
  i32 count;
  i32 capacity;

  inline void init(memory_arena_t* memory, i32 capacity) {
    this->capacity = capacity;
    this->count = 0;
    this->values = PUSH_ARRAY(memory, capacity, T);
  }
  inline T& operator[] (i32 index) { return this->values[index]; }
  inline T* at(i32 index) { return this->values + index; }
  inline T* push(T val) {
    T* result = this->values + this->count++;
    assert_(this->count <= this->capacity);
    *result = val;
    return result;
  }
  inline T* push_many(i32 count) {
    T* result = this->values + this->count;
    this->count += count;
    assert_(this->count <= this->capacity);
    ZERO_ARRAY(result, count);
    return result;
  }
  inline T pop() {
    assert_(this->count);
    T result = this->values[--this->count];
    return result;
  }
  inline i32 push_unassigned() {
    i32 result = this->count++;
    assert_(this->count <= this->capacity);
    return result;
  }
  inline void set(i32 index, T val) { this->values[index] = val; }
};

template <class T>
struct array {
  T *values;
  i32 count;

  inline void init(memory_arena_t* memory, i32 count) {
    this->count = count;
    this->values = PUSH_ARRAY(memory, count, T);
  }
  inline T operator[](i32 index) { return this->values[index]; }
  inline T *at(i32 index) { return this->values + index; }
  inline void set(i32 index, T val) { this->values[index] = val; }
};

template <class T1, class T2>
struct tuple2 {
  T1 first;
  T2 second;
};

template <class T1, class T2, class T3>
struct tuple3 {
  T1 first;
  T2 second;
  T3 third;
};

template <class T>
struct pool_obj {
  T obj;
  b32 freed;
};

template <class T>
inline b32 is_freed(T* ptr) {
  return ((pool_obj<T>*)ptr)->freed;
}

template <class T>
struct pool {
  T* values;
  vec<i32> freed;
  i32 size;
  i32 capacity;

  inline void init(memory_arena_t* memory, i32 capacity) {
    this->capacity = capacity;
    this->size = 0;
    this->values = PUSH_ARRAY(memory, capacity, T);
    this->freed.init(memory, capacity);
  }

  inline T*
  acquire() {
    T* result;
    if (freed.count) {
      result = values + freed[--freed.count];
      ZERO_ARRAY(result, 1);
    } else {
      result = values + size++;
      assert_(size <= capacity);
    }
    return result;
  }

  inline array<T>
  acquire_many(i32 count) {
    array<T> result;

    b32 found = false;
    i32 start = 0;
    if (count == 1) {
      result.values = acquire();
    } else {
      for (int i = 1; i < freed.count; ++i) {
        if (freed[i] != (freed[i-1] + 1)) {
          start = i;
        }
        if ((i - start) == (count - 1)) {
          result.values = values + start;
          ZERO_ARRAY(result.values, count);

          memmove(freed.values + start,
                  freed.values + start + count,
                  (freed.count - (start + count)) * sizeof(i32));
          freed.count -= count;

          found = true;
        }
      }

      if (!found) {
        result.values = values + size;
        size += count;
        assert_(size <= capacity);
      }
    }

    result.count = count;

    return result;
  }

  inline T*
  get(i32 index) {
    return values + index;
  }

  inline i32
  index_of(T* val) {
    T* loc = (T*)val;
    return (i32)(loc - values);
  }

  inline void
  free(T* val) {
    i32 index = index_of(val);
    freed.push(index);
  }

  inline void
  free_many(T* val, i32 count) {
    i32 index = index_of(val);

    for (int i = 0; i < count; ++i) {
      freed.push(index + i);
    }
  }
};

template <class T>
struct iterable_pool {
  pool_obj<T>* values;
  vec<i32> freed;
  i32 size;
  i32 capacity;

  inline void init(memory_arena_t* memory, i32 capacity) {
    this->capacity = capacity;
    this->size = 0;
    this->values = PUSH_ARRAY(memory, capacity, pool_obj<T>);
    this->freed.init(memory, capacity);
  }

  inline T*
  acquire() {
    pool_obj<T>* result;
    if (freed.count) {
      result = values + freed[--freed.count];
      ZERO_STRUCT(result->obj);
    } else {
      result = values + size++;
      assert_(size <= capacity);
    }
    result->freed = false;

    return &result->obj;
  }

  inline array<pool_obj<T>>
  acquire_many(i32 count) {
    array<pool_obj<T>> result;
    i32 start = 0;
    b32 found = false;

    if (count == 1) {
      result.values = acquire();
    } else {
      for (int i = 1; i < freed.count; ++i) {
        if (freed[i] != (freed[i-1] + 1)) {
          start = i;
        }
        if ((i - start) == (count - 1)) {
          result.values = values + start;
          ZERO_ARRAY(result.values, count);

          memmove(freed.values + start,
                  freed.values + start + count,
                  (freed.count - (start + count)) * sizeof(i32));
          freed.count -= count;

          found = true;
        }
      }

      if (!found) {
        result.values = values + size;
        size += count;
        assert_(size <= capacity);
      }
    }
    result.count = count;

    return result;
  }

  inline T* 
  try_get(i32 index) {
    pool_obj<T>* result = values + index;
    if (!result->freed) {
      return &result->obj;
    } else {
      return 0;
    }
  }

  inline i32
  index_of(T* val) {
    pool_obj<T>* loc = (pool_obj<T>*)val;
    return (i32)(loc - values);
  }

  inline void 
  free(T* val) {
    i32 index = index_of(val);
    freed.push(index);
    values[index].freed = true;
  }

  inline void 
  free_many(T* val, i32 count) {
    i32 index = index_of(val);

    for (int i = index; i < index + count; ++i) {
      values[i].freed = true;
      freed.push(i);
    }
  }

  inline void
  allocate(memory_arena_t* memory, i32 capacity) {
    this->capacity = capacity;
    values = (pool_obj<T> *)_push_size(memory, capacity * sizeof(pool_obj<T>));
    freed.capacity = capacity / 2;
    freed.values = (i32 *)_push_size(memory, freed.capacity * sizeof(i32));
  }
};

#endif //PHYSICA_TYPEDEFS_H

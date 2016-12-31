//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_HASHMAP_H
#define PHYSICA_HASHMAP_H

#include "typedefs.h"
#include "limits.h"

struct hash_pair_ {
    void* val;
    u64 key;
};

struct hashmap_ {
    u32 capacity;
    u32 count;
    hash_pair_* pairs;
};

template<class T>
struct hashpair {
    T val;
    u64 key;
};

template<class T>
struct hashmap {
    array<hashpair<T>> pairs;
};

inline u64 i64_bits_to_u64(i64 val) {
    union { i64 i; u64 u; } c;
    c.i = val;
    return c.u;
}

inline i64 u64_bits_to_i64(u64 val) {
    union { i64 i; u64 u; } c;
    c.u = val;
    return c.i;
}

inline u64 _rotl64(u64 val, u32 degree) {
    return (val << degree) | (val >> (64 - degree));
}

inline u32 _get_hashed_key(u64 h) {
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccd;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53;
    h ^= h >> 33;

    return (u32)h;
}

inline b32 _slot_occupied(hashmap_* hm, i32 slot) {
    return hm->pairs[slot].val != 0;
}

inline i32 _find_slot(hashmap_* hm, u64 k) {
    i32 i = (i32)(_get_hashed_key(k) % hm->capacity);
    while (_slot_occupied(hm,i) && hm->pairs[i].key != k) {
        ++i;
        i %= hm->capacity;
    }
    return i;
}

inline void* get_hash_item(hashmap_* hm, u64 k) {
    i32 i = _find_slot(hm, k);
    return hm->pairs[i].val;
}

inline void* get_hash_item(hashmap_* hm, i64 k) {
    return get_hash_item(hm, i64_bits_to_u64(k));
}

template<class T>
inline b32 _slot_occupied(hashmap<T>* hm, i32 slot) {
    return hm->pairs[slot].key != 0;
}

template<class T>
inline i32 _find_slot(hashmap<T>* hm, u64 k) {
    u32 i = _get_hashed_key(k) % (u32)hm->pairs.count;
    while (_slot_occupied(hm,(i32)i) && hm->pairs[(i32)i].key != k) {
        ++i;
        i %= (u32)hm->pairs.count;
    }
    assert_(i <= INT_MAX);
    return (i32)i;
}

template<class T>
inline T* get_hash_item(hashmap<T>* hm, u64 k) {
    i32 i = _find_slot(hm, k);
    hashpair<T>* pair = hm->pairs.at(i);
    if (pair->key) {
        return &pair->val;
    } else {
        return (T*)0;
    }
}

template<class T>
inline T* get_hash_item(hashmap<T>* hm, i64 k) {
    return get_hash_item(hm, i64_bits_to_u64(k));
}

template<class T>
inline T get_hash_item_value(hashmap<T>* hm, u64 k) {
    return *get_hash_item(hm, k);
}

template<class T>
inline T get_hash_item_value(hashmap<T>* hm, i64 k) {
    return *get_hash_item(hm, k);
}

// inline void set_hash_item(hashmap_* hm, u64 k, void* val) {
//     i32 i = _find_slot(hm, k);
//     hm->pairs[i].key = k;
//     hm->pairs[i].val = val;
//     hm->count++;
//     assert_(hm->count < (hm->capacity / 2));
// }

template<class T>
inline T* set_hash_item(hashmap<T>* hm, u64 k, T val) {
    assert_(k);
    i32 i = _find_slot(hm, k);
    hm->pairs.at(i)->key = k;
    hm->pairs.at(i)->val = val;
    return &hm->pairs.at(i)->val;
}

template<class T>
inline T* set_hash_item(hashmap<T>* hm, i64 k, T val) {
    return set_hash_item(hm, i64_bits_to_u64(k), val);
}

inline void remove_hash_item(hashmap_* hm, u64 k) {
    i32 i = _find_slot(hm, k);
    if (!_slot_occupied(hm, i)) {
        return;
    }
    i32 j = i;
    b32 loop = (b32) true;
    while (loop) {
        hm->pairs[i].key = 0;
        hm->pairs[i].val = {0};

        i32 l = 0;
        do {
            j++;
            j %= hm->capacity;
            if (!_slot_occupied(hm, j)) {
                loop = (b32) false;
                break;
            }

            u32 l_key = _get_hashed_key(hm->pairs[j].key);
            l = (i32)(l_key % hm->capacity);
        } while((i <= j) ? ((i< l)&&(l <=j)) : (i< l)||(l <=j));

        hm->pairs[i] = hm->pairs[j];
        i = j;
    }
    hm->count--;
}

inline void remove_hash_item(hashmap_* hm, i64 k) {
    remove_hash_item(hm, i64_bits_to_u64(k));
}

template <class T>
inline void remove_hash_item(hashmap<T>* hm, u64 k) {
    i32 i = _find_slot(hm, k);
    if (!_slot_occupied(hm, i)) {
        return;
    }
    i32 j = i;
    b32 loop = (b32) true;
    while (loop) {
        hm->pairs.at(i)->val = 0;

        i32 l = 0;
        do {
            j++;
            j %= hm->pairs.count;
            if (!_slot_occupied(hm, j)) {
                loop = (b32) false;
                break;
            }

            u32 l_key = _get_hashed_key(hm->pairs[j].key);
            l = (i32)(l_key % (u32)hm->pairs.count);
        } while((i <= j) ? ((i< l)&&(l <=j)) : (i< l)||(l <=j));

        hm->pairs[i] = hm->pairs[j];
        i = j;
    }
}

template <class T>
inline void remove_hash_item(hashmap<T>* hm, i64 k) {
    remove_hash_item(hm, i64_bits_to_u64(k));
}

template <class T>
inline void clear_hashmap(hashmap<T>* hm) {
    ZERO_ARRAY(hm->pairs.values, hm->pairs.count);
}

#endif //PHYSICA_HASHMAP_H

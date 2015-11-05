//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_HASHMAP_H
#define PHYSICA_HASHMAP_H

#include "typedefs.h"

struct hash_pair_t {
    void* val;
    u64 key;
};

struct hashmap_t {
    u32 capacity;
    u32 count;
    hash_pair_t* pairs;
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

inline b32 _slot_occupied(hashmap_t* hm, u32 slot) {
    return hm->pairs[slot].val != 0;
}

inline u32 _find_slot(hashmap_t* hm, u64 k) {
    u32 i = _get_hashed_key(k) % hm->capacity;
    while (_slot_occupied(hm,i) && hm->pairs[i].key != k) {
        ++i;
        i %= hm->capacity;
    }
    return i;
}

inline void* get_hash_item(hashmap_t* hm, u64 k) {
    u32 i = _find_slot(hm, k);
    return hm->pairs[i].val;
}

template<class T>
inline b32 _slot_occupied(hashmap<T>* hm, u32 slot) {
    return hm->pairs[slot].key != 0;
}

template<class T>
inline u32 _find_slot(hashmap<T>* hm, u64 k) {
    u32 i = _get_hashed_key(k) % hm->pairs.count;
    while (_slot_occupied(hm,i) && hm->pairs[i].key != k) {
        ++i;
        i %= hm->pairs.count;
    }
    return i;
}

template<class T>
inline T* get_hash_item(hashmap<T>* hm, u64 k) {
    u32 i = _find_slot(hm, k);
    hashpair<T>* pair = hm->pairs.at(i);
    if (pair->key) {
        return &pair->val;
    } else {
        return (T*)0;
    }
}

inline void set_hash_item(hashmap_t* hm, u64 k, void* val) {
    u32 i = _find_slot(hm, k);
    hm->pairs[i].key = k;
    hm->pairs[i].val = val;
    hm->count++;
    assert_(hm->count < (hm->capacity / 2));
}

template<class T>
inline T* set_hash_item(hashmap<T>* hm, u64 k, T val) {
    assert_(k);
    u32 i = _find_slot(hm, k);
    u64 old_k =hm->pairs.at(i)->key;
    hm->pairs.at(i)->key = k;
    hm->pairs.at(i)->val = val;
    return &hm->pairs.at(i)->val;
}

inline void remove_hash_item(hashmap_t* hm, u64 k) {
    u32 i = _find_slot(hm, k);
    if (!_slot_occupied(hm, i)) {
        return;
    }
    u32 j = i;
    b32 loop = (b32) true;
    while (loop) {
        hm->pairs[i].key = 0;
        hm->pairs[i].val = {0};

        u32 l = 0;
        do {
            j++;
            j %= hm->capacity;
            if (!_slot_occupied(hm, j)) {
                loop = (b32) false;
                break;
            }

            l = _get_hashed_key(hm->pairs[j].key);
        } while((i <= j) ? ((i< l)&&(l <=j)) : (i< l)||(l <=j));

        hm->pairs[i] = hm->pairs[j];
        i = j;
    }
    hm->count--;
}

template <class T>
inline void remove_hash_item(hashmap<T>* hm, u64 k) {
    u32 i = _find_slot(hm, k);
    if (!_slot_occupied(hm, i)) {
        return;
    }
    u32 j = i;
    b32 loop = (b32) true;
    while (loop) {
        hm->pairs.at(i)->val = 0;

        u32 l = 0;
        do {
            j++;
            j %= hm->capacity;
            if (!_slot_occupied(hm, j)) {
                loop = (b32) false;
                break;
            }

            l = _get_hashed_key(hm->pairs.at(j)->key);
        } while((i <= j) ? ((i< l)&&(l <=j)) : (i< l)||(l <=j));

        hm->pairs[i] = hm->pairs[j];
        i = j;
    }
    hm->count--;
}

template <class T>
inline void clear_hashmap(hashmap<T>* hm) {
    ZERO_ARRAY(hm->pairs.values, hm->pairs.count);
}

#endif //PHYSICA_HASHMAP_H

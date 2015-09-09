//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_PHYSICA_H
#define PHYSICA_PHYSICA_H

#include "typedefs.h"
#include "sim_entity.h"
#include "hashmap.h"

struct phy_hull_t {
    f32 mass, inv_mass, moment, inv_moment, orientation;

    v2 position;
    v2 relative_position; // relative to body centroid
    array<v2> points; // TODO(doug): better geometry

    // ...
};

struct phy_edge_t {
    v2 normal;
    i32 a;
    i32 b;
    f32 depth;
    u32 polytope_index;
};

struct phy_memory_t {
    u8* base;
    i64 size;
};

struct phy_support_result_t {
    v2 p_a;
    v2 p_b;
    v2 p;
};

struct phy_aabb_t {
    v2 min, max;
};

const u32 PHY_FIXED_FLAG = 1;

struct phy_body_t {
    i64 id; // intended for use outside the physics engine
    u32 flags; // see above
    f32 mass, inv_mass, moment, inv_moment; // (moment of inertia)
    v2 position, velocity;
    f32 orientation, angular_velocity;
    v2 force; // zeroed after integration
    f32 torque; // zeroed after integration
    phy_aabb_t aabb;
    i32 aabb_node_index;
    array<phy_hull_t> hulls;
};

struct phy_collision_t {
    v2 normal;
    f32 depth;
    b32 persistent;
    v2 local_contact_a, local_contact_b;
    v2 world_contact_a, world_contact_b;
    u32 a_index, b_index;
};

const i32 COLLISION_CAPACITY = 2;
struct phy_manifold_t {
    i32 collision_count;
    f32 normal_sum, tangent_sum;
    phy_collision_t collisions[COLLISION_CAPACITY];
};

const i32 LEAF_NODE = -1;

struct phy_aabb_tree_node_t {
    i32 parent;
    union {
        i32 left;
        i32 body_index;
    };
    union {
        i32 right;
        i32 type;
    };
    phy_aabb_t fat_aabb;
};

const v2 FAT_AABB_MARGIN = v2 {0.2f, 0.2f};

struct phy_aabb_tree_t {
    vec<phy_aabb_tree_node_t> nodes;
    vec<i32> dead_nodes;
    array<b32> checked_parents;
    i32 root;
};

struct phy_potential_collision_t {
    i32 a_index;
    i32 b_index;
};

struct phy_state_t {
    vec<phy_body_t> bodies;
    vec<phy_hull_t> hulls;
    vec<v2> points;
    vec<phy_potential_collision_t> potential_collisions;
    vec<phy_collision_t> collisions;
    hashmap<phy_manifold_t> manifold_cache;
    v2 gravity;
    phy_aabb_tree_t aabb_tree;
    f32 time_step, current_time;
};

void phy_set_gravity(phy_memory_t memory, v2 gravity);

void phy_init(phy_memory_t memory);

phy_body_t * phy_add_body(phy_memory_t memory);

array<phy_hull_t> phy_add_hulls(phy_memory_t memory, i32 count);

array<v2> phy_add_points(phy_memory_t memory, i32 count);

phy_collision_t* phy_add_collision(phy_memory_t memory);

phy_collision_t* phy_add_collision(phy_memory_t memory,
                                   phy_collision_t collision);

void phy_update(phy_memory_t memory, f32 dt);

void phy_add_aabb_for_body(phy_memory_t memory, i32 body_index);

// NOTE(doug): user-defined
phy_support_result_t do_support(phy_hull_t* a, phy_hull_t* b, v2 direction);

v2 do_support(phy_hull_t* a, v2 direction);
#endif //PHYSICA_PHYSICA_H

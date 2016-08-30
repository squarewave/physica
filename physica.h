//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_PHYSICA_H
#define PHYSICA_PHYSICA_H

#include "typedefs.h"
#include "sim_entity.h"
#include "hashmap.h"

const i32 LEAF_NODE = -1;
const v2 FAT_AABB_MARGIN = v2 {0.2f, 0.2f};

struct phy_body_t;

struct phy_aabb_t {
    v2 min, max;
};

struct phy_aabb_tree_node_t {
    i32 parent;
    b32 is_asleep;
    union {
        struct {
            i32 type;
            phy_body_t* body;
        };
        struct {
            i32 right;
            i32 left;
        };
    };
    phy_aabb_t fat_aabb;
};

struct phy_aabb_tree_t {
    vec<phy_aabb_tree_node_t> nodes;
    vec<i32> dead_nodes;
    array<b32> checked_parents;
    i32 root;
};

enum hull_type_t {
    HULL_MESH = 0,
    HULL_RECT = 1,
    HULL_FILLET_RECT = 2
};

struct phy_hull_t {
    f32 mass, inv_mass, moment, inv_moment, orientation;

    v2 position;
    v2 relative_position; // relative to body centroid

    i32 type;
    union {
        array<v2> points;               // type == HULL_MESH
        struct {                        // type == HULL_RECT || HULL_FILLET_RECT
            f32 width, height, fillet;
        };
    };

    // ...
};

struct phy_edge_t {
    v2 normal;
    i32 a;
    i32 b;
    f32 depth;
    i32 polytope_index;
};

struct phy_support_result_t {
    v2 p_a;
    v2 p_b;
    v2 p;
};

const u32 PHY_FIXED_FLAG        = 0x01;
const u32 PHY_WEIGHTLESS_FLAG   = 0x02;
const u32 PHY_INCORPOREAL_FLAG  = 0x04;
const u32 PHY_GROUND_FLAG       = 0x08;
const u32 PHY_CHARACTER_FLAG    = 0x10;

struct phy_collision_t {
    v2 normal;
    f32 depth;
    b32 persistent;
    v2 local_contact_a, local_contact_b;
    v2 world_contact_a, world_contact_b;
    phy_body_t *a, *b;
};

const i32 COLLISION_CAPACITY = 2;
struct phy_manifold_t {
    i32 collision_count;
    f32 normal_sum, tangent_sum;
    phy_collision_t collisions[COLLISION_CAPACITY];
};

struct phy_potential_collision_t {
    phy_body_t* a;
    phy_body_t* b;
};

struct entity_ties_t {
    i64 id;
    i32 type;
};

struct phy_body_t {
    entity_ties_t entity;
    u32 flags;
    f32 mass, inv_mass, moment, inv_moment;
    v2 position, velocity;
    f32 orientation, angular_velocity;
    v2 gravity_normal;
    v2 force; // zeroed after integration
    f32 torque; // zeroed after integration
    phy_aabb_t aabb;
    i32 aabb_node_index;
    array<phy_hull_t> hulls;
};

struct phy_state_t {
    iterable_pool<phy_body_t> bodies;
    pool<phy_hull_t> hulls;
    pool<v2> points;
    array<v2> previous_velocities;
    array<f32> previous_angular_velocities;
    vec<phy_potential_collision_t> potential_collisions;
    vec<phy_collision_t> collisions;
    hashmap<phy_manifold_t> manifold_cache;
    v2 gravity;
    phy_aabb_tree_t aabb_tree;
    f32 time_step, current_time;
};

struct ray_intersect_t {
    b32 intersecting;
    f32 depth;
};

struct ray_body_intersect_t {
    phy_body_t* body;
    f32 depth;
};

phy_aabb_tree_node_t* aabb_insert_node(phy_aabb_tree_t* tree,
                                       i32 parent_index,
                                       phy_aabb_t fat_aabb,
                                       phy_body_t body,
                                       b32 is_asleep);

void aabb_remove_node(phy_aabb_tree_t *tree, i32 index);

b32 aabb_are_intersecting(phy_aabb_t a, phy_aabb_t b);

b32 aabb_is_contained_in(phy_aabb_t inner, phy_aabb_t outer);

void phy_set_gravity(phy_state_t* state, v2 gravity);

phy_state_t phy_init(memory_arena_t* memory);

phy_body_t* phy_add_block(phy_state_t* state,
                          v2 center,
                          v2 diagonal,
                          f32 mass,
                          f32 orientation);

phy_body_t* phy_add_fillet_block(phy_state_t* state,
                                 v2 center,
                                 v2 diagonal,
                                 f32 fillet, 
                                 f32 mass,
                                 f32 orientation);

phy_body_t * phy_add_body(phy_state_t* state);

void phy_remove_body(phy_state_t* state);

array<phy_hull_t> phy_add_hulls(phy_state_t* state, i32 count);

array<v2> phy_add_points(phy_state_t* state, i32 count);

phy_collision_t* phy_add_collision(phy_state_t* state);

phy_collision_t* phy_add_collision(phy_state_t* state, phy_collision_t collision);

void phy_update(phy_state_t* state, hashmap<entity_ties_t>* collision_map, f32 dt);

void phy_add_aabb_for_body(phy_state_t* state, phy_body_t* body);

// NOTE(doug): user-defined
phy_support_result_t do_support(phy_hull_t* a, phy_hull_t* b, v2 direction);

v2 do_support(phy_hull_t* a, v2 direction);

ray_intersect_t ray_segment_intersect(v2 p, v2 d, v2 a, v2 b);

ray_intersect_t ray_hull_intersect(v2 p, v2 d, phy_hull_t* hull);

ray_body_intersect_t ray_cast(phy_state_t* state,
                              v2 p,
                              v2 d,
                              u32 required_flags = 0,
                              phy_body_t* exclude = 0);

ray_body_intersect_t ray_cast_from_body(phy_state_t* state,
                                        phy_body_t* self,
                                        f32 width,
                                        v2 d,
                                        u32 required_flags = 0);
#endif //PHYSICA_PHYSICA_H

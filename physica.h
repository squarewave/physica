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

struct phy_body_;

struct phy_aabb_ {
    v2 min, max;
};

struct phy_aabb_tree_node_ {
    i32 parent;
    b32 is_asleep;
    union {
        struct {
            i32 type;
            phy_body_* body;
        };
        struct {
            i32 right;
            i32 left;
        };
    };
    phy_aabb_ fat_aabb;
};

struct phy_aabb_tree_ {
    vec<phy_aabb_tree_node_> nodes;
    vec<i32> dead_nodes;
    array<b32> checked_parents;
    i32 root;
};

enum hull_type_ {
    HULL_MESH = 0,
    HULL_RECT = 1,
    HULL_FILLET_RECT = 2
};

struct phy_hull_ {
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

struct phy_edge_ {
    v2 normal;
    i32 a;
    i32 b;
    f32 depth;
    i32 polytope_index;
};

struct phy_support_result_ {
    v2 p_a;
    v2 p_b;
    v2 p;
};

const u32 PHY_FIXED_FLAG        = 0x01;
const u32 PHY_WEIGHTLESS_FLAG   = 0x02;
const u32 PHY_INCORPOREAL_FLAG  = 0x04;
const u32 PHY_GROUND_FLAG       = 0x08;
const u32 PHY_CHARACTER_FLAG    = 0x10;

struct phy_collision_ {
    v2 normal;
    f32 depth;
    b32 persistent;
    v2 local_contact_a, local_contact_b;
    v2 world_contact_a, world_contact_b;
    phy_body_ *a, *b;
};

const i32 COLLISION_CAPACITY = 2;
struct phy_manifold_ {
    i32 collision_count;
    f32 normal_sum, tangent_sum;
    phy_collision_ collisions[COLLISION_CAPACITY];
};

struct phy_potential_collision_ {
    phy_body_* a;
    phy_body_* b;
};

struct entity_ties_ {
    i64 id;
    i32 type;
};

struct phy_body_ {
    entity_ties_ entity;
    u32 flags;
    f32 mass, inv_mass, moment, inv_moment;
    v2 position, velocity;
    f32 orientation, angular_velocity;
    v2 gravity_normal;
    v2 force; // zeroed after integration
    f32 torque; // zeroed after integration
    phy_aabb_ aabb;
    i32 aabb_node_index;
    array<phy_hull_> hulls;
};

struct phy_state_ {
    iterable_pool<phy_body_> bodies;
    pool<phy_hull_> hulls;
    pool<v2> points;
    array<v2> previous_velocities;
    array<f32> previous_angular_velocities;
    vec<phy_potential_collision_> potential_collisions;
    vec<phy_collision_> collisions;
    hashmap<phy_manifold_> manifold_cache;
    v2 gravity;
    phy_aabb_tree_ aabb_tree;
    f32 time_step, current_time;
};

struct ray_intersect_ {
    b32 intersecting;
    f32 depth;
};

struct ray_body_intersect_ {
    phy_body_* body;
    f32 depth;
};

phy_aabb_tree_node_* aabb_insert_node(phy_aabb_tree_* tree,
                                       i32 parent_index,
                                       phy_aabb_ fat_aabb,
                                       phy_body_ body,
                                       b32 is_asleep);

void aabb_remove_node(phy_aabb_tree_ *tree, i32 index);

b32 aabb_are_intersecting(phy_aabb_ a, phy_aabb_ b);

b32 aabb_is_contained_in(phy_aabb_ inner, phy_aabb_ outer);

void phy_set_gravity(phy_state_* state, v2 gravity);

phy_state_ phy_init(memory_arena_* memory);

phy_body_* phy_add_block(phy_state_* state,
                          v2 center,
                          v2 diagonal,
                          f32 mass,
                          f32 orientation);

phy_body_* phy_add_fillet_block(phy_state_* state,
                                 v2 center,
                                 v2 diagonal,
                                 f32 fillet, 
                                 f32 mass,
                                 f32 orientation);

phy_body_ * phy_add_body(phy_state_* state);

void phy_remove_body(phy_state_* state);

array<phy_hull_> phy_add_hulls(phy_state_* state, i32 count);

array<v2> phy_add_points(phy_state_* state, i32 count);

phy_collision_* phy_add_collision(phy_state_* state);

phy_collision_* phy_add_collision(phy_state_* state, phy_collision_ collision);

void phy_update(phy_state_* state, hashmap<entity_ties_>* collision_map, f32 dt);

void phy_add_aabb_for_body(phy_state_* state, phy_body_* body);

// NOTE(doug): user-defined
phy_support_result_ do_support(phy_hull_* a, phy_hull_* b, v2 direction);

v2 do_support(phy_hull_* a, v2 direction);

ray_intersect_ ray_segment_intersect(v2 p, v2 d, v2 a, v2 b);

ray_intersect_ ray_hull_intersect(v2 p, v2 d, phy_hull_* hull);

ray_body_intersect_ ray_cast(phy_state_* state,
                              v2 p,
                              v2 d,
                              u32 required_flags = 0,
                              phy_body_* exclude = 0);

ray_body_intersect_ ray_cast_from_body(phy_state_* state,
                                        phy_body_* self,
                                        f32 width,
                                        v2 d,
                                        u32 required_flags = 0);
#endif //PHYSICA_PHYSICA_H

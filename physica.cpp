//
// Created by doug on 4/19/15.
//

#include <assert.h>
#include "physica.h"
#include "float.h"
#include "game.h"
#include "game_render.h"

void
phy_init(phy_memory_t memory) {
    phy_state_t* state = (phy_state_t*)memory.base;

    state->time_step = 0.005f;

    u8* previous_loc = memory.base + sizeof(phy_state_t);
    i32 previous_size = 0;
#define __PUSH_STATE_INIT(o, cnt, prop) ({\
    previous_loc = previous_loc + previous_size;\
    previous_size = sizeof(*o.values) * (cnt);\
    *((u8**)&o.values) = previous_loc;\
    o.prop = (cnt);\
})

#define __PUSH_STATE_INIT_VEC(a,b) __PUSH_STATE_INIT(a,b,capacity);
#define __PUSH_STATE_INIT_ARRAY(a,b) __PUSH_STATE_INIT(a,b,count);

    i32 body_capacity = 2000;

    __PUSH_STATE_INIT_VEC(state->bodies, 2000);
    __PUSH_STATE_INIT_ARRAY(state->previous_velocities, 2000);
    __PUSH_STATE_INIT_ARRAY(state->previous_angular_velocities, 2000);
    __PUSH_STATE_INIT_VEC(state->hulls, 4000);
    __PUSH_STATE_INIT_VEC(state->points, 16000);
    __PUSH_STATE_INIT_VEC(state->collisions, 2000);
    __PUSH_STATE_INIT_VEC(state->aabb_tree.nodes, 8000);
    __PUSH_STATE_INIT_ARRAY(state->aabb_tree.checked_parents, 8000);
    __PUSH_STATE_INIT_VEC(state->aabb_tree.dead_nodes, 8000);
    __PUSH_STATE_INIT_VEC(state->potential_collisions, 8000);
    __PUSH_STATE_INIT_ARRAY(state->manifold_cache.pairs, 8000);

    void* free_memory = (void*)(previous_loc + previous_size);
    const i32 free_memory_size = 2 * 1024 * 1024; //2MB
    assert(((i64)free_memory - (i64)(memory.base)) + free_memory_size < memory.size);

    state->free_memory = free_memory;
    state->free_memory_size = free_memory_size;
}

phy_aabb_t
get_union(phy_aabb_t a, phy_aabb_t b) {
    phy_aabb_t result;
    result.min.x = fmin(a.min.x, b.min.x);
    result.min.y = fmin(a.min.y, b.min.y);
    result.max.x = fmax(a.max.x, b.max.x);
    result.max.y = fmax(a.max.y, b.max.y);
    return result;
}

ray_intersect_t
ray_hull_intersect(v2 p, v2 d, phy_hull_t* hull) {
    ray_intersect_t result;
    result.intersecting = false;
    if (hull->type == HULL_FILLET_RECT) {
        return result;
    }
    m2x2 rotation = get_rotation_matrix(hull->orientation);
    for (int i = 0; i < hull->points.count; ++i) {

        v2 a = hull->position + rotation * hull->points[(i == 0 ? hull->points.count : i) - 1];
        v2 b = hull->position + rotation * hull->points[i];
        ray_intersect_t r = ray_segment_intersect(p,d,a,b);

        if (!r.intersecting || (result.intersecting && result.depth < r.depth)) { continue; }

        result = r;
    }
    return result;
}

inline v2 do_support_mesh(phy_hull_t* hull, v2 direction) {
    f32 greatest = -FLT_MAX;
    f32 last = -FLT_MAX;
    m2x2 rotation = get_rotation_matrix(hull->orientation);
    v2 result = {};
    for (int i = 0; i < hull->points.count; ++i) {
        v2 transformed = rotation * hull->points[i] + hull->position;
        f32 test = dot(transformed, direction);
        if (test > greatest) {
            greatest = test;
            result = transformed;
        }
        last = test;
    }
    return result;
}

inline v2 do_support_rect(phy_hull_t* hull, v2 direction) {
    v2 local_direction = rotate(direction, -hull->orientation);
    v2 local_result;
    if (local_direction.x > 0.0f) {
        local_result.x = hull->width * 0.5f;
    } else {
        local_result.x = -hull->width * 0.5f;
    }
    if (local_direction.y > 0.0f) {
        local_result.y = hull->height * 0.5f;
    } else {
        local_result.y = -hull->height * 0.5f;
    }

    return hull->position + rotate(local_result, hull->orientation);
}

inline v2 do_support_rect_fillet(phy_hull_t* hull, v2 direction) {
    v2 local_direction = rotate(direction, -hull->orientation);
    v2 local_result;
    if (local_direction.x > 0.0f) {
        local_result.x = (hull->width * 0.5f) - hull->fillet;
    } else {
        local_result.x = (-hull->width * 0.5f) + hull->fillet;
    }
    if (local_direction.y > 0.0f) {
        local_result.y = (hull->height * 0.5f) - hull->fillet;
    } else {
        local_result.y = (-hull->height * 0.5f) + hull->fillet;
    }

    v2 fillet = normalize(local_direction) * (hull->fillet);
    return hull->position + rotate(local_result + fillet, hull->orientation);
}

v2
do_support(phy_hull_t* hull, v2 direction) {
    switch (hull->type) {
        case HULL_MESH: {
            return do_support_mesh(hull, direction);
        } break;
        case HULL_RECT: {
            return do_support_rect(hull, direction);
        } break;
        case HULL_FILLET_RECT: {
            return do_support_rect_fillet(hull, direction);
        } break;
    }
}

phy_support_result_t
do_support(phy_hull_t* a, phy_hull_t* b, v2 direction) {
    phy_support_result_t result;
    result.p_a = do_support(a, direction);
    result.p_b = do_support(b, -direction);
    result.p = result.p_a - result.p_b;
    return result;
}

f32
area(phy_aabb_t aabb) {
    v2 vec = aabb.max - aabb.min;
    return vec.x * vec.y;
}

b32
aabb_are_intersecting(phy_aabb_t a, phy_aabb_t b) {
    if (a.max.x < b.min.x || b.max.x < a.min.x) return false;
    if (a.max.y < b.min.y || b.max.y < a.min.y) return false;
    return true;
}

b32 aabb_is_contained_in(phy_aabb_t inner, phy_aabb_t outer) {
    return inner.max.x <= outer.max.x && inner.max.y <= outer.max.y &&
           inner.min.x >= outer.min.x && inner.min.y >= outer.min.y;
}

b32 aabb_is_contained_in(v2 p, phy_aabb_t aabb) {
    return p.x >= aabb.min.x && p.y >= aabb.min.y &&
           p.x <= aabb.max.x && p.y <= aabb.max.y;
}

phy_aabb_tree_node_t*
aabb_insert_node(phy_aabb_tree_t* tree,
            i32 parent_index,
            phy_aabb_t fat_aabb,
            i32 body_index) {
    phy_aabb_tree_node_t* result;
    b32 found_leaf = false;
    while (!found_leaf) {
        phy_aabb_tree_node_t *parent = tree->nodes.at(parent_index);
        if (parent->type == LEAF_NODE) {
            i32 left_index;
            if (tree->dead_nodes.count) {
                left_index = tree->dead_nodes[--tree->dead_nodes.count];
            }
            else {
                left_index = tree->nodes.push_unassigned();
            }
            i32 right_index;
            if (tree->dead_nodes.count) {
                right_index = tree->dead_nodes[--tree->dead_nodes.count];
            }
            else {
                right_index = tree->nodes.push_unassigned();
            }
            phy_aabb_tree_node_t *left = tree->nodes.at(left_index);
            phy_aabb_tree_node_t *right = tree->nodes.at(right_index);
            i32 parent_body_index = parent->body_index;
            parent->left = left_index;
            parent->right = right_index;

            left->parent = parent_index;
            left->fat_aabb = parent->fat_aabb;
            left->body_index = parent_body_index;
            left->type = LEAF_NODE;

            right->parent = parent_index;
            right->fat_aabb = fat_aabb;
            right->body_index = body_index;
            right->type = LEAF_NODE;

            found_leaf = true;
        } else {
            phy_aabb_tree_node_t *left = tree->nodes.at(parent->left);
            phy_aabb_tree_node_t *right = tree->nodes.at(parent->right);

            phy_aabb_t left_union = get_union(left->fat_aabb, fat_aabb);
            phy_aabb_t right_union = get_union(right->fat_aabb, fat_aabb);

            parent_index = area(left_union) < area(right_union)
                           ? parent->left
                           : parent->right;
        }
    }

    result = tree->nodes.at(parent_index);

    while (parent_index != -1) {
        phy_aabb_tree_node_t *parent = tree->nodes.at(parent_index);
        phy_aabb_tree_node_t *left = tree->nodes.at(parent->left);
        phy_aabb_tree_node_t *right = tree->nodes.at(parent->right);
        parent->fat_aabb = get_union(left->fat_aabb, right->fat_aabb);
        parent_index = parent->parent;
    }

    return result;
}

void aabb_remove_node(phy_aabb_tree_t *tree, i32 index) {
    i32 parent_index = tree->nodes[index].parent;
    if (parent_index == -1) {
        assert(tree->nodes.count == 1);
        tree->nodes.count = 0;
        tree->dead_nodes.count = 0;
    } else {
        phy_aabb_tree_node_t *parent = tree->nodes.at(parent_index);
        i32 sibling_index = (parent->left == index) ? parent->right
                                                    : parent->left;
        phy_aabb_tree_node_t *sibling = tree->nodes.at(sibling_index);
        if (parent->parent != -1) {
            phy_aabb_tree_node_t *grandparent = tree->nodes.at(parent->parent);
            sibling->parent = parent->parent;
            i32 parent_was_right = grandparent->right == parent_index;
            if (parent_was_right) {
                grandparent->right = sibling_index;
            } else {
                grandparent->left = sibling_index;
            }
        } else {
            tree->root = sibling_index;
            tree->nodes.at(tree->root)->parent = -1;
        }
        tree->dead_nodes.push(parent_index);
        tree->dead_nodes.push(index);
    }

    while (parent_index != -1) {
        phy_aabb_tree_node_t *parent = tree->nodes.at(parent_index);
        phy_aabb_tree_node_t *left = tree->nodes.at(parent->left);
        phy_aabb_tree_node_t *right = tree->nodes.at(parent->right);
        parent->fat_aabb = get_union(left->fat_aabb, right->fat_aabb);
        parent_index = parent->parent;
    }
}

phy_aabb_t
get_aabb(phy_body_t *body) {
    phy_aabb_t result;
    result.min = v2 {FLT_MAX, FLT_MAX};
    result.max = v2 {-FLT_MAX, -FLT_MAX};
    m2x2 rotation = get_rotation_matrix(body->orientation);
    for (int i = 0; i < body->hulls.count; ++i) {
        phy_hull_t *hull = body->hulls.at(i);
        if (hull->type == HULL_MESH) {
            for (int j = 0; j < hull->points.count; ++j) {
                v2 p = hull->position + rotation * hull->points[j];
                if (p.x < result.min.x) { result.min.x = p.x; }
                if (p.y < result.min.y) { result.min.y = p.y; }
                if (p.x > result.max.x) { result.max.x = p.x; }
                if (p.y > result.max.y) { result.max.y = p.y; }
            }   
        } else {
            result.min.x = do_support(hull, v2 {-1.0f, 0.0f}).x;
            result.max.x = do_support(hull, v2 {1.0f, 0.0f}).x;
            result.min.y = do_support(hull, v2 {0.0f, -1.0f}).y;
            result.max.y = do_support(hull, v2 {0.0f, 1.0f}).y;
        }
    }
    return result;
}

inline void
check_children(phy_state_t *state,
               phy_aabb_tree_t *tree,
               i32 index,
               phy_aabb_tree_node_t *node) {
    if (!tree->checked_parents[index]) {
        tree->checked_parents.values[index] = true;
        find_broad_phase_collisions(state, node->left, node->right);
    }
}

ray_intersect_t
ray_segment_intersect(v2 p, v2 d, v2 a, v2 b) {
    v2 v_1 = p - a;
    v2 v_2 = b - a;
    v2 v_3 = perp(d);
    f32 t_1 = flt_cross(v_2, v_1) / dot(v_2, v_3);
    f32 t_2 = dot(v_1, v_3) / dot(v_2, v_3);
    ray_intersect_t result;
    result.intersecting = t_2 >= 0.0f && t_2 <= 1.0f && t_1 >= 0.0f;

    result.depth = t_1;
    return result;
}

inline ray_intersect_t
ray_aabb_intersect(v2 p, v2 d, phy_aabb_t aabb) {
    ray_intersect_t result;
    result.intersecting = false;

    if (aabb_is_contained_in(p, aabb)) {
        result.intersecting = true;
        result.depth = 0;
    } else {
        if (p.x <= aabb.min.x) {
            f32 t = (aabb.min.x - p.x) / d.x;
            f32 y = p.y + t * d.y;

            if (t > 0.0f && y >= aabb.min.y && y <= aabb.max.y) {
                result.intersecting = true;
                result.depth = t;
            }
        } else if(!result.intersecting && p.x >= aabb.max.x) {
            f32 t = (aabb.max.x - p.x) / d.x;
            f32 y = p.y + t * d.y;

            if (t > 0.0f && y >= aabb.min.y && y <= aabb.max.y) {
                result.intersecting = true;
                result.depth = t;
            }
        }

        if(!result.intersecting && p.y <= aabb.min.y) {
            f32 t = (aabb.min.y - p.y) / d.y;
            f32 x = p.x + t * d.x;

            if (t > 0.0f && x >= aabb.min.x && x <= aabb.max.x) {
                result.intersecting = true;
                result.depth = t;
            }
        } else if (!result.intersecting && p.y >= aabb.max.y) {
            f32 t = (aabb.max.y - p.y) / d.y;
            f32 x = p.x + t * d.x;

            if (t > 0.0f && x >= aabb.min.x && x <= aabb.max.x) {
                result.intersecting = true;
                result.depth = t;
            }
        }
    }
    return result;
}

inline ray_intersect_t
ray_body_intersect(v2 p, v2 d, phy_body_t* body) {
    ray_intersect_t result;
    result.intersecting = false;
    for (int i = 0; i < body->hulls.count; ++i) {
        ray_intersect_t r = ray_hull_intersect(p,d,body->hulls.at(i));

        if (!r.intersecting || (result.intersecting && result.depth < r.depth)) { continue; }

        result = r;
    }
    return result;
}

ray_body_intersect_t
ray_cast_from_body(phy_memory_t memory, phy_body_t* self, f32 width, v2 d) {

    phy_state_t* state = (phy_state_t*)memory.base;
    phy_aabb_tree_t* tree = &state->aabb_tree;

    ray_body_intersect_t result = {0};

    i32* stack = (i32*)state->free_memory;
    for (int i = 0; i < 2; ++i) {
        v2 pd = perp(d);
        v2 p = i ?
            self->position + (0.5f * width * pd) :
            self->position + (-0.5f * width * pd);

        i32 stack_index = 0;
        stack[stack_index++] = tree->root;

        while (stack_index >= 0) {
            phy_aabb_tree_node_t* node = tree->nodes.at(stack[--stack_index]);
            ray_intersect_t r = ray_aabb_intersect(p, d, node->fat_aabb);
            if (!r.intersecting || (result.body && result.depth < r.depth)) {
                continue;
            }

            if (node->type == LEAF_NODE) {
                phy_body_t* body = state->bodies.at(node->body_index);
                if (body == self) { continue; }
                r = ray_body_intersect(p, d, body);

                if (!r.intersecting || (result.body && result.depth < r.depth)) { continue; }

                result.body = body;
                result.depth = r.depth;
            } else {
                stack[stack_index++] = node->left;
                stack[stack_index++] = node->right;
            }
        }
    }

    return result;
}

void
find_broad_phase_collisions(phy_state_t* state) {
    TIMED_BLOCK(find_broad_phase_collisions);

    phy_aabb_tree_t* tree = &state->aabb_tree;
    state->potential_collisions.count = 0;
    if (tree->nodes.count == 0) {
        return;;
    }
    phy_aabb_tree_node_t* root = tree->nodes.at(tree->root);
    if (root->type == LEAF_NODE) {
        return;
    }
    for (int i = 0; i < tree->checked_parents.count; ++i) {
        tree->checked_parents.values[i] = false;
    }

    i32* stack = (i32*)state->free_memory;
    i32 stack_index = -1;
    stack[++stack_index] = root->left;
    stack[++stack_index] = root->right;

    while (stack_index >= 0) {
        assert(stack_index % 2 == 1);
        i32 a_index = stack[stack_index--];
        i32 b_index = stack[stack_index--];

        phy_aabb_tree_node_t *a = tree->nodes.at(a_index);
        phy_aabb_tree_node_t *b = tree->nodes.at(b_index);

        if (a->type == LEAF_NODE) {
            if (b->type == LEAF_NODE) {
                if (!(state->bodies[a->body_index].flags & PHY_FIXED_FLAG) ||
                    !(state->bodies[b->body_index].flags & PHY_FIXED_FLAG)) {

                    phy_aabb_t aabb_a = state->bodies[a->body_index].aabb;
                    phy_aabb_t aabb_b = state->bodies[b->body_index].aabb;
                    if (aabb_are_intersecting(aabb_a, aabb_b)) {
                        phy_potential_collision_t collision;
                        if (a->body_index < b->body_index) {
                            collision.a_index = a->body_index;
                            collision.b_index = b->body_index;
                        } else {
                            collision.a_index = b->body_index;
                            collision.b_index = a->body_index;
                        }
                        state->potential_collisions.push(collision);
                    }
                }
            } else {
                if (!tree->checked_parents[b_index]) {
                    tree->checked_parents.values[b_index] = true;
                    stack[++stack_index] = b->left;
                    stack[++stack_index] = b->right;
                }

                if (aabb_are_intersecting(a->fat_aabb, b->fat_aabb)) {
                    stack[++stack_index] = a_index;
                    stack[++stack_index] = b->left;
                    stack[++stack_index] = a_index;
                    stack[++stack_index] = b->right;
                }
            }
        } else {
            if (b->type == LEAF_NODE) {

                if (!tree->checked_parents[a_index]) {
                    tree->checked_parents.values[a_index] = true;
                    stack[++stack_index] = a->left;
                    stack[++stack_index] = a->right;
                }

                if (aabb_are_intersecting(a->fat_aabb, b->fat_aabb)) {
                    stack[++stack_index] = b_index;
                    stack[++stack_index] = a->left;
                    stack[++stack_index] = b_index;
                    stack[++stack_index] = a->right;
                }
            } else {
                if (!tree->checked_parents[a_index]) {
                    tree->checked_parents.values[a_index] = true;
                    stack[++stack_index] = a->left;
                    stack[++stack_index] = a->right;
                }
                if (!tree->checked_parents[b_index]) {
                    tree->checked_parents.values[b_index] = true;
                    stack[++stack_index] = b->left;
                    stack[++stack_index] = b->right;
                }

                if (aabb_are_intersecting(a->fat_aabb, b->fat_aabb)) {

                    stack[++stack_index] = b->left;
                    stack[++stack_index] = a->left;
                    stack[++stack_index] = b->right;
                    stack[++stack_index] = a->right;

                    stack[++stack_index] = b->right;
                    stack[++stack_index] = a->left;
                    stack[++stack_index] = b->left;
                    stack[++stack_index] = a->right;
                }
            }
        }
    }
}

phy_body_t*
phy_add_body(phy_memory_t memory) {
    phy_state_t* state = (phy_state_t*)memory.base;
    assert(state->bodies.count < state->bodies.capacity);
    return state->bodies.values + state->bodies.count++;
}

array<phy_hull_t>
phy_add_hulls(phy_memory_t memory, i32 count) {
    array<phy_hull_t> result;
    phy_state_t* state = (phy_state_t*)memory.base;
    assert(state->hulls.count + count <= state->hulls.capacity);
    result.values = state->hulls.values + state->hulls.count;
    result.count = count;
    state->hulls.count += count;
    return result;
}

array<v2>
phy_add_points(phy_memory_t memory, i32 count) {
    array<v2> result;
    phy_state_t* state = (phy_state_t*)memory.base;
    assert(state->points.count + count <= state->points.capacity);

    result.values = state->points.values + state->points.count;
    result.count = count;

    state->points.count += count;

    return result;
}

phy_collision_t*
phy_add_collision(phy_state_t* state) {
    assert(state->collisions.count < state->collisions.capacity);
    return state->collisions.values + state->collisions.count++;
}

phy_collision_t*
phy_add_collision(phy_state_t* state, phy_collision_t collision) {
    assert(state->collisions.count < state->collisions.capacity);
    phy_collision_t* result =
            state->collisions.values + state->collisions.count++;
    *result = collision;
    return result;
}

void
update_hulls(phy_body_t* body) {
    m2x2 rotation = get_rotation_matrix(body->orientation);
    for (int i = 0; i < body->hulls.count; ++i) {
        phy_hull_t* hull = body->hulls.at(i);
        hull->position = body->position + rotation * hull->relative_position;
        hull->orientation = body->orientation;
    }
}

v2
do_simplex(phy_support_result_t support,
           phy_support_result_t* simplex,
           i32* simplex_count) {
    v2 result;
    switch (*simplex_count) {
        case 1: {
            v2 ab = support.p - simplex[0].p;
            simplex[1] = support;
            *simplex_count += 1;
            result = triple(ab, ab, simplex[0].p);
        } break;
        case 2: {
            //        (support) c
            //                 / \
            //              o?/   \ o?
            //               /  o? \
            // (simplex[0]) a-------b (simplex[1])

            v2 a = simplex[0].p;
            v2 b = simplex[1].p;
            v2 c = support.p;

            v2 ac = c - a;
            // perpendicular away from b
            v2 ac_p = triple(ac, ac, b);
            f32 dot_acp = dot(ac_p, c);
            if (dot_acp == 0.0f) {
                // origin is on ac - return a collision and let EPA handle
                // whether or not we care
                *simplex_count += 1;
                simplex[2] = support;
                result = v2 {0};
            } else if (dot_acp > 0.0f) {
                // origin may be inside simplex - keep checking
                v2 bc = c - b;
                // perpendicular away from a
                v2 bc_p = triple(bc, bc, a);
                f32 dot_bcp = dot(bc_p, c);
                if (dot_bcp >= 0.0f) {
                    // origin is on bc or inside simplex
                    *simplex_count += 1;
                    simplex[2] = support;
                    result = v2 {0};
                } else {
                    // origin is on outside of bc - replace a with c and check
                    // perpendicular to bc away from c
                    simplex[0] = support;
                    result = bc_p;
                }
            } else {
                // origin is on outside of ac - replace b with c and check
                // perpendicular to ac away from b
                simplex[1] = support;
                result = ac_p;
            }
        } break;
        default: assert(false);
    }
    return result;
}

// must guarantee to populate simplex with 3 points if it returns true
b32
do_gjk(phy_hull_t* a, phy_hull_t* b, phy_support_result_t* simplex) {
    TIMED_BLOCK(do_gjk);
    v2 d = v2 {1,1};
    phy_support_result_t support = do_support(a, b, d);
    if (dot(support.p, d) <= 0.0f) {
        return false;
    }
    simplex[0] = support;
    d = -d;
    i32 simplex_count = 1;
    for (int i = 0; i < 20; ++i) {
        support = do_support(a, b, d);
        if (dot(support.p, d) <= 0.0f) {
            return false;
        }

        d = do_simplex(support, simplex, &simplex_count);
        if (simplex_count == 3) {
            return true;
        }
    }
    assert(false);
}

phy_edge_t
find_closest_edge_to_origin(phy_support_result_t* polytope,
                            u32 vertex_count) {
    phy_edge_t result = {0};
    result.depth = FLT_MAX;
    result.normal = {0};
    u32 polytope_index = 0;
    for (int i = 0; i < vertex_count; ++i) {
        u32 next = (i + 1) % vertex_count;
        v2 a = polytope[i].p;
        v2 b = polytope[next].p;

        v2 e = b - a;
        v2 n = triple(e, a, e);
        n = normalize(n);

        f32 test_distance = dot(n, a);
        if (test_distance < result.depth) {
            result.depth = test_distance;
            result.normal = n;
            result.polytope_index = next;
            result.a = i;
            result.b = next;
        }
    }

    return result;
}

inline void
insert_into_polytope(phy_support_result_t* polytope,
                     u32* vertex_count,
                     u32 index,
                     phy_support_result_t vertex) {
    for (int i = *vertex_count; i > index; --i) {
        polytope[i] = polytope[i - 1];
    }
    polytope[index] = vertex;
    *vertex_count += 1;
}

inline void
reorder_simplex(phy_support_result_t* simplex) {
    if (dot(perp(simplex[1].p - simplex[0].p), simplex[2].p) < 0.0f) {
        phy_support_result_t temp = simplex[0];
        simplex[0] = simplex[1];
        simplex[1] = temp;
    }
}

b32
do_epa(phy_hull_t* a,
       phy_hull_t* b,
       phy_support_result_t* polytope,
       phy_edge_t* result) {

    reorder_simplex(polytope);
    u32 vertex_count = 3;
    const f32 threshold = 0.0001f;

    for (int i = 0; i < 20; i++) {
        phy_edge_t edge = find_closest_edge_to_origin(polytope,
                                                      vertex_count);
        phy_support_result_t support = do_support(a, b, edge.normal);

        f32 d = dot(support.p, edge.normal);
        if (abs(d - edge.depth) < threshold) {
            if (edge.depth > 0.0f) {
                *result = edge;
                return true;
            } else {
                return false;
            }
        } else {
            insert_into_polytope(polytope,
                                 &vertex_count,
                                 edge.polytope_index,
                                 support);
        }
    }
    assert(false);
}

inline bool
try_find_collision(phy_state_t* state, u32 a_index, u32 b_index,
                   i32 hull_index_a, i32 hull_index_b,
                   phy_collision_t *collision) {

    phy_body_t* a = state->bodies.at(a_index);
    phy_body_t* b = state->bodies.at(b_index);

    if (a->flags & PHY_FIXED_FLAG && b->flags & PHY_FIXED_FLAG) {
        return false;
    }

    phy_support_result_t simplex[32] = {0};
    phy_hull_t *a_hull = a->hulls.values + hull_index_a;
    phy_hull_t *b_hull = b->hulls.values + hull_index_b;
    if (!do_gjk(a_hull, b_hull, simplex)) {
        return false;
    }

    phy_edge_t edge;
    if (!do_epa(a_hull, b_hull, simplex, &edge)) {
        return false;
    }

    collision->a_index = a_index;
    collision->b_index = b_index;
    collision->normal = edge.normal;
    collision->depth = edge.depth;
    phy_support_result_t start = simplex[edge.a];
    phy_support_result_t fin = simplex[edge.b];

    v2 p = start.p;
    v2 e = fin.p - p;
    v2 n = edge.normal;

    f32 t = -flt_cross(p, n) / flt_cross(e, n);

    v2 ae = fin.p_a - start.p_a;
    v2 be = fin.p_b - start.p_b;

    collision->world_contact_a = start.p_a + t * ae;
    collision->world_contact_b = start.p_b + t * be;

    collision->local_contact_a = rotate(collision->world_contact_a - a->position, -a->orientation);
    collision->local_contact_b = rotate(collision->world_contact_b - b->position, -b->orientation);
    return true;
}

f32
solve_velocity_constraint(phy_body_t *a,
                 phy_body_t *b,
                 f32 zeta,
                 v6 &jacobian,
                 f32 lambda_min,
                 f32 lambda_max,
                 f32* lambda_sum) {
    v6 state = {
        a->velocity.x, a->velocity.y, a->angular_velocity,
        b->velocity.x, b->velocity.y, b->angular_velocity
    };
    v6 j2 = {
        jacobian.vals[0] * a->inv_mass,
        jacobian.vals[1] * a->inv_mass,
        jacobian.vals[2] * a->inv_moment,
        jacobian.vals[3] * b->inv_mass,
        jacobian.vals[4] * b->inv_mass,
        jacobian.vals[5] * b->inv_moment
    };

    f32 effective_mass = dot(jacobian, j2);
    f32 lagrangian = -(dot(state, jacobian) + zeta) / effective_mass;
    f32 new_sum = fclamp(*lambda_sum + lagrangian, lambda_min, lambda_max);
    lagrangian = new_sum - *lambda_sum;
    *lambda_sum = new_sum;

    v6 delta_state = lagrangian * j2;

    a->velocity += v2 {delta_state.vals[0], delta_state.vals[1]};
    a->angular_velocity += delta_state.vals[2];   

    b->velocity += v2 {delta_state.vals[3], delta_state.vals[4]};
    b->angular_velocity += delta_state.vals[5];   

    return lagrangian;
}

void
phy_add_aabb_for_body(phy_memory_t memory,
                      i32 body_index) {
    phy_state_t* state = (phy_state_t*)memory.base;
    phy_aabb_tree_t* tree = &state->aabb_tree;

    phy_body_t *body = state->bodies.at(body_index);
    i32 index = -1;
    phy_aabb_t aabb = get_aabb(body);
    phy_aabb_t fat_aabb = phy_aabb_t {
            aabb.min - FAT_AABB_MARGIN,
            aabb.max + FAT_AABB_MARGIN
    };
    body->aabb = aabb;
    if (tree->nodes.count == 0) { // this is our root
        index = tree->dead_nodes.count
                ? tree->dead_nodes[--tree->dead_nodes.count]
                : (tree->nodes.count++);
        phy_aabb_tree_node_t* node = tree->nodes.at(index);
        tree->root = index;
        node->parent = -1;
        node->body_index = body_index;
        node->fat_aabb = fat_aabb;
        node->type = LEAF_NODE;
        body->aabb_node_index = index;
    } else {
        phy_aabb_tree_node_t *parent = aabb_insert_node(tree, tree->root, fat_aabb, body_index);
        i32 left = parent->left;
        i32 right = parent->right;

        state->bodies.at(tree->nodes.at(left)->body_index)->aabb_node_index = left;
        state->bodies.at(tree->nodes.at(right)->body_index)->aabb_node_index = right;
    }
}

inline b32
is_stale(phy_state_t* state, phy_collision_t* c) {
    auto a = state->bodies.at(c->a_index);
    auto b = state->bodies.at(c->b_index);
    m3x3 transform_a =
            get_translation_matrix(a->position) *
            get_rotation_matrix_3x3(a->orientation);
    m3x3 transform_b =
            get_translation_matrix(b->position) *
            get_rotation_matrix_3x3(b->orientation);

    v2 new_world_a = transform_a * c->local_contact_a;
    v2 new_world_b = transform_b * c->local_contact_b;

    static i32 depth_c = 0;

    f32 depth = dot(new_world_b - new_world_a, c->normal);
    if (depth > 0.0f) {
        return true;
    }
    c->depth = depth;

    const f32 threshold_sq = 0.002f;

    static i32 length_c = 0;

    if (length_squared(new_world_a - c->world_contact_a) > threshold_sq) {
        return true;
    }

    if (length_squared(new_world_b - c->world_contact_b) > threshold_sq) {
        return true;
    }


    // add_debug_point(new_world_a, 0xffffffff);
    // add_debug_point(c->world_contact_a, 0x00000000);

    return false;
}

inline b32 are_same(phy_collision_t *first, phy_collision_t *second) {
    const f32 threshold_sq = 0.002f;
    
    assert(first->a_index == second->a_index);
    v2 distance_a = first->local_contact_a - second->local_contact_a;
    v2 distance_b = first->local_contact_b - second->local_contact_b;
    return length_squared(distance_a) < threshold_sq &&
           length_squared(distance_b) < threshold_sq;
}

inline i32
which_has_max_depth(phy_state_t* state,
                    phy_collision_t* collisions,
                    i32 count) {
    f32 max = -FLT_MAX;
    i32 max_index = -1;
    for (int i = 0; i < count; ++i) {
        if (collisions[i].depth > max) {
            max = collisions[i].depth;
            max_index = i;
        }
    }
    return max_index;
}

// gets the furthest collision from the collision with the max depth
inline i32
which_is_furthest(phy_state_t* state,
                  phy_collision_t* collisions,
                  i32 count,
                  i32 max_depth_index) {
    f32 max = -FLT_MAX;
    i32 max_index = -1;
    phy_collision_t max_depth = collisions[max_depth_index];
    for (int i = 0; i < count; ++i) {
        if (i == max_depth_index) { continue; }
        phy_collision_t collision = collisions[i];
        f32 total_distance_sq_ish = length_squared(collision.world_contact_a -
                                        max_depth.world_contact_a) +
                                    length_squared(collision.world_contact_b -
                                        max_depth.world_contact_b);
        if (total_distance_sq_ish > max) {
            max = total_distance_sq_ish;
            max_index = i;
        }
    }
    assert(max_index != -1);
    return max_index;
}

phy_manifold_t*
get_collision_manifold(phy_state_t *state,
                       phy_collision_t *collision,
                       phy_body_t *a, phy_body_t *b) {
    TIMED_BLOCK(find_narrow_phase_collisions);
    u64 a_64 = (u64)collision->a_index;
    u64 b_64 = (u64)collision->b_index;
    assert(a_64 < b_64);
    u64 hash_key = (b_64 << 32) | a_64;
    phy_manifold_t* manifold =
            get_hash_item(&state->manifold_cache, hash_key);

    phy_manifold_t new_manifold = {0};
    if (manifold) {
        new_manifold.normal_sum = manifold->normal_sum;
        new_manifold.tangent_sum = manifold->tangent_sum;
        phy_collision_t potential_collisions[3];
        i32 potential_collision_index = 0;
        potential_collisions[potential_collision_index++] = *collision;
        for (int j = 0; j < manifold->collision_count; ++j) {
            phy_collision_t* c = &manifold->collisions[j];
            if (!is_stale(state, c) && !are_same(c, collision)) {
                c->persistent = true;
                potential_collisions[potential_collision_index++] = *c;
            }
        }
        if (potential_collision_index == 1) {
            new_manifold.collisions[new_manifold.collision_count++] =
                potential_collisions[--potential_collision_index];
        } else if (potential_collision_index == 2) {
            new_manifold.collisions[new_manifold.collision_count++] =
                    potential_collisions[--potential_collision_index];
            new_manifold.collisions[new_manifold.collision_count++] =
                    potential_collisions[--potential_collision_index];
        } else {
            assert(potential_collision_index == 3);
            i32 max_depth = 0;
            i32 furthest = which_is_furthest(state,
                                             potential_collisions,
                                             3,
                                             max_depth);
            new_manifold.collisions[new_manifold.collision_count++] =
                    potential_collisions[max_depth];
            new_manifold.collisions[new_manifold.collision_count++] =
                    potential_collisions[furthest];
        }
    } else {
        new_manifold.collision_count = 1;
        new_manifold.collisions[0] = *collision;
    }

    return set_hash_item(&state->manifold_cache,
                             hash_key,
                             new_manifold);
}

void
warm_start(phy_state_t* state, phy_manifold_t* manifold) {
    if (manifold->collision_count == 0) {
        return;
    }

    phy_body_t* a = state->bodies.at(manifold->collisions[0].a_index);
    phy_body_t* b = state->bodies.at(manifold->collisions[0].b_index);

    for (int i = 0; i < manifold->collision_count; ++i) {
        phy_collision_t* collision = &manifold->collisions[i];
        if (collision->persistent) {
            v2 normal = collision->normal;
            v2 tangent = cross(normal, 1.0f);

            v2 impulse = manifold->normal_sum * normal + manifold->tangent_sum * tangent;
            a->velocity = a->velocity - a->inv_mass * impulse;
            b->velocity = b->velocity + b->inv_mass * impulse;
            a->angular_velocity = a->angular_velocity -
                    a->inv_moment * flt_cross(collision->local_contact_a, impulse);
            b->angular_velocity = b->angular_velocity +
                    b->inv_moment * flt_cross(collision->local_contact_b, impulse);
        }
    }
}

void
find_narrow_phase_collisions(phy_state_t* state) {
    TIMED_FUNC();

    for (int i = 0; i < state->potential_collisions.count; ++i) {
        phy_potential_collision_t potential_collision =
                state->potential_collisions[i];
        int a_index = potential_collision.a_index;
        int b_index = potential_collision.b_index;
        phy_body_t* a = state->bodies.values + a_index;
        phy_body_t* b = state->bodies.values + b_index;
        bool found = false;
        phy_collision_t collision = {0};
        for (int j = 0; j < a->hulls.count && !found; ++j) {
            for (int k = 0; k < b->hulls.count && !found; ++k) {
                found = try_find_collision(state, a_index, b_index, j, k, &collision);
            }
        }
        if (!found) {
            continue;
        }
        phy_add_collision(state, collision);
    }
}

void
pre_solve_velocity_constraints(phy_state_t* state) {
    TIMED_FUNC();

    for (int i = 0; i < state->collisions.count; ++i) {
        phy_collision_t *collision = state->collisions.at(i);
        phy_body_t *a = state->bodies.at(collision->a_index);
        phy_body_t *b = state->bodies.at(collision->b_index);

        phy_manifold_t *manifold = get_collision_manifold(state,
                                                          collision,
                                                          a, b);
        manifold->normal_sum = 0.0f;
        manifold->tangent_sum = 0.0f;
    }
}

void
solve_velocity_constraints(phy_state_t* state, f32 dt) {
    TIMED_FUNC();

    for (int i = 0; i < state->collisions.count; ++i) {
        phy_collision_t *collision = state->collisions.at(i);
        phy_body_t *a = state->bodies.at(collision->a_index);
        phy_body_t *b = state->bodies.at(collision->b_index);


        phy_manifold_t *manifold = get_collision_manifold(state,
                                                          collision,
                                                          a, b);

        const f32 restitution = 0.8f;
        const f32 friction_coefficient = 0.1f;
        const f32 baumgarte = 0.2f;
        const f32 penetration_slop = 0.002f;
        const f32 restitution_slop = 0.02f;

//        warm_start(state, manifold);

        for (int j = 0; j < manifold->collision_count; ++j) {
            auto c = &manifold->collisions[j];

            v2 n = c->normal;
            v2 ra = c->world_contact_a - a->position;
            v2 rb = c->world_contact_b - b->position;
            v2 va = a->velocity;
            v2 vb = b->velocity;
            f32 omega_a = a->angular_velocity;
            f32 omega_b = b->angular_velocity;

            v2 relative_velocity = vb - va +
                                   cross(rb, omega_b) -
                                   cross(ra, omega_a);

            f32 bf = -(baumgarte / dt) *
                     fmax(c->depth - penetration_slop, 0) +
                     restitution *
                     fmin(dot(relative_velocity, n) + restitution_slop, 0);

            v6 jacobian = {
                    -n.x, -n.y, -flt_cross(ra, n),
                    n.x, n.y, flt_cross(rb, n)
            };

            solve_velocity_constraint(a, b, bf, jacobian, 0, FLT_MAX,
                             &manifold->normal_sum);

            v2 t = normalize(triple(n, relative_velocity, n));
            if (length_squared(t) > 0) {
                v6 tangent_jacobian = {
                        -t.x, -t.y, -flt_cross(ra, t),
                        t.x, t.y, flt_cross(rb, t)
                };

                solve_velocity_constraint(a, b,
                                 0,
                                 tangent_jacobian,
                                 -friction_coefficient *
                                 manifold->normal_sum,
                                 friction_coefficient *
                                 manifold->normal_sum,
                                 &manifold->tangent_sum);
            }   
        }


        *(state->previous_velocities.at(collision->a_index)) = a->velocity;
        *(state->previous_velocities.at(collision->b_index)) = b->velocity;
        *(state->previous_angular_velocities.at(collision->a_index)) = a->angular_velocity;
        *(state->previous_angular_velocities.at(collision->b_index)) = b->angular_velocity;

    }
}

void
integrate_velocities(phy_state_t* state, f32 dt) {
    TIMED_BLOCK(integrate_velocities);

    for (int i = 0; i < state->bodies.count; ++i) {
        phy_body_t* body = &state->bodies.values[i];
        *(state->previous_velocities.at(i)) = body->velocity;
        *(state->previous_angular_velocities.at(i)) = body->angular_velocity;
        if (!(body->flags & PHY_FIXED_FLAG)) {
            body->force += state->gravity * body->mass;
        }
        v2 accel = body->force * body->inv_mass;
        f32 angular_accel = body->torque * body->inv_moment;
        body->velocity = body->velocity + accel * dt;
        body->angular_velocity = body->angular_velocity + angular_accel * dt;

        body->velocity *= 1.0f / (1.0f + dt * 0.01f);
        body->angular_velocity *= 1.0f / (1.0f + dt * 0.01f);
    }
}

void
integrate_positions(phy_state_t* state, f32 dt) {
    TIMED_FUNC();

    for (int i = 0; i < state->bodies.count; ++i) {
        phy_body_t* body = state->bodies.at(i);

        v2 avg_velocity = (body->velocity + state->previous_velocities[i]) * 0.5f;
        f32 avg_angular_velocity =
            (body->angular_velocity + state->previous_angular_velocities[i]) * 0.5f;

        f32 velocity_threshold = 0.01f;
        if (abs(length_squared(avg_velocity)) > velocity_threshold) {
            body->position = body->position + avg_velocity * dt;
        }
        if (abs(avg_angular_velocity) > velocity_threshold) {
            body->orientation = body->orientation + avg_angular_velocity * dt;
        }

    }
}

void
_phy_update(phy_memory_t memory, f32 dt) {
    TIMED_FUNC();
    // phy_hull_t hull;
    // hull.type = HULL_FILLET_RECT;
    // hull.width = 2.0f;
    // hull.height = 5.0f;
    // hull.fillet = 1.0f;
    // hull.position = v2 {0.0f, 0.0f};
    // for (int i = 0; i < 40; ++i) {
    //     f32 direction = 2.0f * 3.14159f * (40.0f / (f32)i);
    //     v2 point = do_support(&hull, rotate(v2{1.0f,0.0f}, direction));
    //     add_debug_point(point, 0xffffffff);
    // }

    // add_debug_point(do_support(&hull, v2{1.0f,-1.0f}), 0xffff0000);


    // add_debug_point(v2{0.0f,0.0f}, 0xffffffff);

    phy_state_t* state = (phy_state_t*)memory.base;

    find_broad_phase_collisions(state);

    find_narrow_phase_collisions(state);

    integrate_velocities(state, dt);

    pre_solve_velocity_constraints(state);

    const i32 velocity_iterations = 6;
    for (i32 i = 0; i < velocity_iterations; ++i) {
        solve_velocity_constraints(state, dt);
    }

    integrate_positions(state, dt);

    for (int i = 0; i < state->bodies.count; ++i) {
        phy_body_t* body = state->bodies.at(i);
        body->force = v2{0,0};
        body->torque = 0.0f;
        update_hulls(body);
        body->aabb = get_aabb(body);
        phy_aabb_t fat_aabb =
                state->aabb_tree.nodes.at(body->aabb_node_index)->fat_aabb;

        if (body->aabb_node_index == -1) {
            phy_add_aabb_for_body(memory, i);
        } else if (!aabb_is_contained_in(body->aabb, fat_aabb)) {
            aabb_remove_node(&state->aabb_tree, body->aabb_node_index);
            phy_add_aabb_for_body(memory, i);
        }
    }

    state->collisions.count = 0;
}

// void check_aabbs(phy_state_t* state, phy_aabb_tree_node_t* node) {
//     phy_aabb_tree_node_t* left = state->aabb_tree.nodes.at(node->left);
//     phy_aabb_tree_node_t* right = state->aabb_tree.nodes.at(node->right);

//     if (node->type == LEAF_NODE) {
//         assert(aabb_is_contained_in(state->bodies.at(node->body_index)->aabb, node->fat_aabb));
//     } else {
//         assert(aabb_is_contained_in(left->fat_aabb, node->fat_aabb));
//         assert(aabb_is_contained_in(right->fat_aabb, node->fat_aabb));

//         check_aabbs(state, left);
//         check_aabbs(state, right);   
//     }
// }

void
phy_update(phy_memory_t memory, f32 dt) {
    TIMED_BLOCK(phy_update);

    phy_state_t* state = (phy_state_t*)memory.base;

    f32 target_time = state->current_time + dt;
    assert(state->time_step > 0);
    while (state->current_time + state->time_step < target_time) {
        state->current_time += state->time_step;
        _phy_update(memory, state->time_step);
    }

    // check_aabbs(state, state->aabb_tree.nodes.at(state->aabb_tree.root));
}



void
phy_set_gravity(phy_memory_t memory, v2 gravity) {
    phy_state_t *state = (phy_state_t*)memory.base;
    state->gravity = gravity;
}

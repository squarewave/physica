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

    state->time_step = 0.01f;

    i32 body_capacity = 2000;
    state->bodies.values = (phy_body_t*)(memory.base + sizeof(phy_state_t));
    state->bodies.capacity = body_capacity;

    i32 hull_capacity = body_capacity * 2;
    state->hulls.values =
            (phy_hull_t*)(state->bodies.values + state->bodies.capacity);
    state->hulls.capacity = hull_capacity;

    i32 point_capacity = hull_capacity * 4;
    phy_hull_t* end_of_hulls = state->hulls.values + state->hulls.capacity;
    state->points.values = (v2*)end_of_hulls;
    state->points.capacity = point_capacity;

    i32 collision_capacity = body_capacity;
    state->collisions.values = (phy_collision_t*)
            (state->points.values + state->points.capacity);
    state->collisions.capacity = point_capacity;

    i32 node_capacity = body_capacity * 4;
    state->aabb_tree.nodes.values = (phy_aabb_tree_node_t*)
            (state->collisions.values + state->collisions.capacity);
    state->aabb_tree.nodes.capacity = node_capacity;

    state->aabb_tree.checked_parents.count = node_capacity;
    state->aabb_tree.checked_parents.values = (b32*)
            (state->aabb_tree.nodes.values + state->aabb_tree.nodes.capacity);

    state->aabb_tree.dead_nodes.capacity = node_capacity;
    state->aabb_tree.dead_nodes.values = (i32*)
            (state->aabb_tree.checked_parents.values +
             state->aabb_tree.checked_parents.count);

    i32 potential_collision_capacity = body_capacity * 4;
    state->potential_collisions.capacity = potential_collision_capacity;
    state->potential_collisions.values = (phy_potential_collision_t*)
            (state->aabb_tree.dead_nodes.values +
             state->aabb_tree.dead_nodes.capacity);

    i32 collision_manifold_map_capacity = body_capacity * 4;
    state->manifold_cache.pairs.count = collision_manifold_map_capacity;
    state->manifold_cache.pairs.values = (hashpair<phy_manifold_t>*)
            (state->potential_collisions.values +
             state->potential_collisions.capacity);    

    assert(((i64)(state->potential_collisions.values +
            state->potential_collisions.capacity) -
            (i64)(memory.base)) < memory.size);
}

phy_aabb_t
get_aabb(phy_body_t *body) {
    phy_aabb_t result;
    result.min = v2 {FLT_MAX, FLT_MAX};
    result.max = v2 {-FLT_MAX, -FLT_MAX};
    m2x2 rotation = get_rotation_matrix(body->orientation);
    for (int i = 0; i < body->hulls.count; ++i) {
        phy_hull_t *hull = body->hulls.at(i);
        for (int j = 0; j < hull->points.count; ++j) {
            v2 p = hull->position + rotation * hull->points[j];
            if (p.x < result.min.x) { result.min.x = p.x; }
            if (p.y < result.min.y) { result.min.y = p.y; }
            if (p.x > result.max.x) { result.max.x = p.x; }
            if (p.y > result.max.y) { result.max.y = p.y; }
        }
    }
    return result;
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

f32
area(phy_aabb_t aabb) {
    v2 vec = aabb.max - aabb.min;
    return vec.x * vec.y;
}

b32
are_intersecting(phy_aabb_t a, phy_aabb_t b) {
    if (a.max.x < b.min.x || b.max.x < a.min.x) return false;
    if (a.max.y < b.min.y || b.max.y < a.min.y) return false;
    return true;
}

b32 is_contained_in(phy_aabb_t inner, phy_aabb_t outer) {
    return inner.max.x <= outer.max.x && inner.max.y <= outer.max.y &&
           inner.min.x >= outer.min.x && inner.min.y <= outer.min.y;
}

void find_broad_phase_collisions(phy_state_t* state, i32 a_index, i32 b_index);

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

void
find_broad_phase_collisions(phy_state_t* state,
                         i32 a_index, i32 b_index) {
    phy_aabb_tree_t* tree = &state->aabb_tree;
    phy_aabb_tree_node_t *a = tree->nodes.at(a_index);
    phy_aabb_tree_node_t *b = tree->nodes.at(b_index);
    if (a->type == LEAF_NODE) {
        if (b->type == LEAF_NODE) {
            phy_aabb_t aabb_a = state->bodies[a->body_index].aabb;
            phy_aabb_t aabb_b = state->bodies[b->body_index].aabb;
            if (are_intersecting(aabb_a, aabb_b)) {
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
        } else {
            check_children(state, tree, b_index, b);
            find_broad_phase_collisions(state, a_index, b->left);
            find_broad_phase_collisions(state, a_index, b->right);
        }
    } else {
        if (b->type == LEAF_NODE) {
            check_children(state, tree, a_index, a);
            find_broad_phase_collisions(state, b_index, a->left);
            find_broad_phase_collisions(state, b_index, a->right);
        } else {
            check_children(state, tree, a_index, a);
            check_children(state, tree, b_index, b);
            find_broad_phase_collisions(state, b->left, a->left);
            find_broad_phase_collisions(state, b->right, a->left);
            find_broad_phase_collisions(state, b->right, a->right);
            find_broad_phase_collisions(state, b->left, a->right);
        }
    }
}

void
find_broad_phase_collisions(phy_state_t* state) {
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
    find_broad_phase_collisions(state, root->left, root->right);
}

void
insert_node(phy_state_t* state,
            i32 parent_index,
            phy_aabb_t fat_aabb,
            i32 body_index) {
    phy_aabb_tree_t* tree = &state->aabb_tree;
    b32 found_leaf = false;
    while (!found_leaf) {
        phy_aabb_tree_node_t *parent = tree->nodes.at(parent_index);
        if (parent->type == LEAF_NODE) {
            found_leaf = true;
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
            state->bodies.at(left->body_index)->aabb_node_index = left_index;

            right->parent = parent_index;
            right->fat_aabb = fat_aabb;
            right->body_index = body_index;
            right->type = LEAF_NODE;
            state->bodies.at(right->body_index)->aabb_node_index = right_index;

            parent->fat_aabb = get_union(parent->fat_aabb, fat_aabb);
        } else {
            phy_aabb_tree_node_t *left = tree->nodes.at(parent->left);
            phy_aabb_tree_node_t *right = tree->nodes.at(parent->right);

            phy_aabb_t left_union = get_union(left->fat_aabb, fat_aabb);
            phy_aabb_t right_union = get_union(right->fat_aabb, fat_aabb);

            parent->fat_aabb = get_union(parent->fat_aabb, fat_aabb);

            parent_index = area(left_union) < area(right_union)
                           ? parent->left
                           : parent->right;
        }
    }
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
        insert_node(state, tree->root, fat_aabb, body_index);
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

    for (int i = 0; i < 20; i++) {
        phy_edge_t edge = find_closest_edge_to_origin(polytope,
                                                      vertex_count);
        phy_support_result_t support = do_support(a, b, edge.normal);

        f32 d = dot(support.p, edge.normal);
        if (fequals(d, edge.depth)) {
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
solve_constraint(phy_body_t *a,
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
remove_aabb(phy_aabb_tree_t *tree, i32 index) {
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

    f32 depth = dot(new_world_b - new_world_a, c->normal);
    if (depth > 0.0f) {
        return true;
    }
    c->depth = depth;

    const f32 threshold_sq = 0.002f;

    if (length_squared(new_world_a - c->world_contact_a) > threshold_sq) {
        return true;
    }

    if (length_squared(new_world_b - c->world_contact_b) > threshold_sq) {
        return true;
    }

    add_debug_point(new_world_a, 0xffffffff);
    add_debug_point(c->world_contact_a, 0x00000000);

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

void find_narrow_phase_collisions(phy_state_t* state) {
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

void pre_solve_velocity_constraints(phy_state_t* state) {
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

void solve_velocity_constraints(phy_state_t* state, f32 dt) {
    for (int i = 0; i < state->collisions.count; ++i) {
        phy_collision_t *collision = state->collisions.at(i);
        phy_body_t *a = state->bodies.at(collision->a_index);
        phy_body_t *b = state->bodies.at(collision->b_index);

        phy_manifold_t *manifold = get_collision_manifold(state,
                                                          collision,
                                                          a, b);

        const f32 restitution = 0.4f;
        const f32 friction_coefficient = 0.55f;
        const f32 baumgarte = 0.1f;
        const f32 penetration_slop = 0.00f;
        const f32 restitution_slop = 0.02f;

//        warm_start(state, manifold);

        for (int j = 0; j < manifold->collision_count; ++j) {
            auto c = &manifold->collisions[j];

            v2 n = c->normal;
            v2 ra = c->local_contact_a;
            v2 rb = c->local_contact_b;
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

            solve_constraint(a, b, bf, jacobian, 0, FLT_MAX,
                             &manifold->normal_sum);

            v2 t = normalize(triple(n, relative_velocity, n));
            if (length_squared(t) > 0) {
                v6 tangent_jacobian = {
                        -t.x, -t.y, -flt_cross(ra, t),
                        t.x, t.y, flt_cross(rb, t)
                };

                solve_constraint(a, b,
                                 0,
                                 tangent_jacobian,
                                 -friction_coefficient *
                                 manifold->normal_sum,
                                 friction_coefficient *
                                 manifold->normal_sum,
                                 &manifold->tangent_sum);
            }
        }
    }
}

void
_phy_update(phy_memory_t memory, f32 dt) {
    phy_state_t* state = (phy_state_t*)memory.base;

    find_broad_phase_collisions(state);

    find_narrow_phase_collisions(state);

    for (int i = 0; i < state->bodies.count; ++i) {
        phy_body_t* body = &state->bodies.values[i];
        if (!(body->flags & FIXED_FLAG)) {
            body->force += state->gravity * body->mass;
        }
        v2 accel = body->force * body->inv_mass;
        f32 angular_accel = body->torque * body->inv_moment;
        body->velocity = body->velocity + accel * dt;
        body->angular_velocity = body->angular_velocity + angular_accel * dt;
    }

    const i32 velocity_iterations = 4;
    pre_solve_velocity_constraints(state);
    for (i32 i = 0; i < velocity_iterations; ++i) {
        solve_velocity_constraints(state, dt);
    }

    state->collisions.count = 0;

    for (int i = 0; i < state->bodies.count; ++i) {
        phy_body_t* body = state->bodies.at(i);
        body->position = body->position + body->velocity * dt;

        body->orientation = body->orientation + body->angular_velocity * dt;
        body->force = v2{0,0};
        body->torque = 0.0f;
        update_hulls(body);
        body->aabb = get_aabb(body);
        phy_aabb_t fat_aabb =
                state->aabb_tree.nodes.at(body->aabb_node_index)->fat_aabb;

        if (body->aabb_node_index == -1) {
            phy_add_aabb_for_body(memory, i);
        } else if (!is_contained_in(body->aabb, fat_aabb)) {
            remove_aabb(&state->aabb_tree, body->aabb_node_index);
            phy_add_aabb_for_body(memory, i);
        }
    }
}

void
phy_update(phy_memory_t memory, f32 dt) {
    phy_state_t* state = (phy_state_t*)memory.base;

    f32 target_time = state->current_time + dt;
    assert(state->time_step > 0);
    while (state->current_time + state->time_step < target_time) {
        state->current_time += state->time_step;
        _phy_update(memory, state->time_step);
    }
}

void phy_set_gravity(phy_memory_t memory, v2 gravity) {
    phy_state_t *state = (phy_state_t*)memory.base;
    state->gravity = gravity;
}

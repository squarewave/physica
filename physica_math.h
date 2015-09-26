//
// Created by doug on 4/16/15.
//

#ifndef PHYSICA_PHYSICA_MATH_H
#define PHYSICA_PHYSICA_MATH_H
#include "emmintrin.h"
#include <math.h>
const f32 fPI = 3.14159265358979323846264338327950288f;

union v2 {
    struct {
        f32 x, y;
    };
    f32 e[2];
};

union v3 {
    struct {
        f32 x,y,z;
    };
    struct {
        f32 r,g,b;
    };
    f32 e[3];
};

struct row2_t {
    f32 c1, c2;
};

struct m2x2 {
    row2_t r1, r2;
};

struct row3_t {
    f32 c1, c2, c3;
};

struct m3x3 {
    union {
        struct {
            row3_t r1, r2, r3;
        };
        f32 vals[9];
    };
};

struct v6 {
    f32 vals[6];
};

inline f32 are_opposite_signs(f32 lhs, f32 rhs) {
    return (lhs > 0.0f && rhs < 0.0f) || (lhs < 0.0f && rhs > 0.0f);
}

inline i32 sfloor(f32 val) {
    return (val < 0.0f) ? ((i32)val - 1) : (i32)val;
}

inline f32 smod(f32 numer, f32 denom) {
    return numer - (sfloor(numer / denom) * denom);
}

inline i32 smod(i32 numer, i32 denom) {
    i32 quot = numer / denom;
    if (numer < 0) {
        numer -= (quot - 1) * denom;
    }
    return numer % denom;
}

inline u32 ufloor(f32 val) {
    return (u32)val;
}

inline u32 uceil(f32 val) {
    return ufloor(val) + 1;
}

inline u32 uround(f32 val) {
    return (u32)(val + 0.5f);
}

inline f32 fclamp(f32 val, f32 min, f32 max) {
    return (val < min) ? min : ((val > max) ? max : val);
}

inline f32 f32max(f32 lhs, f32 rhs) {
    return lhs < rhs ? rhs : lhs;
}

inline f32 f32min(f32 lhs, f32 rhs) {
    return lhs > rhs ? rhs : lhs;
}

inline f32 sticky(f32 val, f32 stick, f32 margin) {
    return (val < (stick + margin) && val > (stick - margin)) ? stick : val;
}

inline v2 v2_from_ints(i32 x, i32 y) {
    v2 result;
    result.x = (f32)x;
    result.y = (f32)y;
    return result;
}

inline v2 operator*(f32 lhs, v2 rhs) {
    v2 result;

    result.x = lhs*rhs.x;
    result.y = lhs*rhs.y;

    return result;
}

inline v3 operator*(f32 lhs, v3 rhs) {
    v3 result;

    result.x = lhs*rhs.x;
    result.y = lhs*rhs.y;
    result.z = lhs*rhs.z;

    return result;
}

inline v2 operator/(v2 lhs, f32 rhs) {
    v2 result;

    result.x = lhs.x/rhs;
    result.y = lhs.y/rhs;

    return result;
}

inline v2 operator*(v2 rhs, f32 lhs) {
    v2 result = lhs*rhs;

    return result;
}

inline v2 &
operator*=(v2 &rhs, f32 lhs) {
    rhs = lhs * rhs;

    return(rhs);
}

inline v2 operator-(v2 lhs) {
    v2 result;

    result.x = -lhs.x;
    result.y = -lhs.y;

    return result;
}

inline v2 operator+(v2 lhs, v2 rhs) {
    v2 result;

    for (int i = 0; i < 2; ++i) {
        result.e[i] = lhs.e[i] + rhs.e[i];
    }

    return result;
}

inline v3 operator+(v3 lhs, v3 rhs) {
    v3 result;
    for (int i = 0; i < 3; ++i) {
        result.e[i] = lhs.e[i] + rhs.e[i];
    }
    return result;
}

inline v2 & operator+=(v2 &lhs, v2 rhs) {
    lhs = lhs + rhs;

    return(lhs);
}

inline v2 operator-(v2 lhs, v2 rhs) {
    v2 result;

    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;

    return result;
}

inline f32 abs(f32 val) {
    return (val > 0) ? val : -val;
}

inline b32 fequals(f32 lhs, f32 rhs) {
    const f32 epsilon = 0.00001f;
    return (abs(lhs - rhs) < epsilon);
}

inline f32 dot(v2 lhs, v2 rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

inline f32 length_squared(v2 val) {
    return val.x * val.x + val.y * val.y;
}

inline f32 length(v2 val) {
    return sqrt(length_squared(val));
}

inline v2 normalize(v2 val) {
    f32 l = length(val);
    return !fequals(l, 0.0f) ? (1.0f / l) * val : v2{0};
}

inline f32 atanv(v2 val) {
    return atan2(val.y, val.x);
}

inline f32 atan2(f32 y, f32 x) {
    return atan2((f64)y, (f64)x);
}

inline f32 sin(f32 val) {
    return sin((f64)val);
}

inline f32 cos(f32 val) {
    return cos((f64)val);
}

inline v2 operator* (m2x2 lhs, v2 rhs) {
    v2 result;
    result.x = lhs.r1.c1 * rhs.x + lhs.r1.c2 * rhs.y;
    result.y = lhs.r2.c1 * rhs.x + lhs.r2.c2 * rhs.y;
    return result;
}

inline m2x2 get_rotation_matrix(f32 theta) {
    return m2x2 {{cos(theta), -sin(theta)}, {sin(theta), cos(theta)}};
}

inline v2 rotate(v2 val, f32 theta) {
    m2x2 r = get_rotation_matrix(theta);
    return r * val;
}

inline v2 v2_from_theta(f32 theta) {
    m2x2 r = m2x2 {{cos(theta), -sin(theta)}, {sin(theta), cos(theta)}};
    return r * (v2 {1,0});
}

inline v3 v3_from_ints(i32 x, i32 y, i32 z = 1) {
    v3 result;
    result.x = (f32)x;
    result.y = (f32)y;
    result.z = (f32)z;
    return result;
}

inline m3x3 operator* (m3x3 lhs, m3x3 rhs) {
    m3x3 result;
#define __ROW_MULT(row,col) lhs.row.c1 * rhs.r1.col +\
            lhs.row.c2 * rhs.r2.col + lhs.row.c3 * rhs.r3.col;
    result.r1.c1 = __ROW_MULT(r1,c1);
    result.r1.c2 = __ROW_MULT(r1,c2);
    result.r1.c3 = __ROW_MULT(r1,c3);
    result.r2.c1 = __ROW_MULT(r2,c1);
    result.r2.c2 = __ROW_MULT(r2,c2);
    result.r2.c3 = __ROW_MULT(r2,c3);
    result.r3.c1 = __ROW_MULT(r3,c1);
    result.r3.c2 = __ROW_MULT(r3,c2);
    result.r3.c3 = __ROW_MULT(r3,c3);
    return result;
}

inline v3 operator* (m3x3 lhs, v3 rhs) {
    __m128 x = _mm_set1_ps(rhs.x);
    __m128 y = _mm_set1_ps(rhs.y);
    __m128 z = _mm_set1_ps(rhs.z);
    __m128 c1 = _mm_set_ps(lhs.r1.c1,lhs.r2.c1,lhs.r3.c1,0.0f);
    __m128 c2 = _mm_set_ps(lhs.r1.c2,lhs.r2.c2,lhs.r3.c2,0.0f);
    __m128 c3 = _mm_set_ps(lhs.r1.c3,lhs.r2.c3,lhs.r3.c3,0.0f);
    __m128 r = _mm_add_ps(_mm_mul_ps(c1, x), _mm_add_ps(_mm_mul_ps(c2, y), _mm_mul_ps(c3,z)));
    return *((v3*)&r);
}

inline v2 perp(v2 val) {
    return v2 {-val.y, val.x};
}

inline f32 flt_cross(v2 lhs, v2 rhs) {
    return lhs.x * rhs.y - lhs.y * rhs.x;
}

inline v2 cross(v2 lhs, f32 rhs) {
    return v2 {lhs.y * rhs, -lhs.x * rhs};
}

inline v3 cross(v3 lhs, v3 rhs) {
    return v3 {
            lhs.y * rhs.z - lhs.z * rhs.y,
            lhs.z * rhs.x - lhs.x * rhs.z,
            lhs.x * rhs.y - lhs.y * rhs.x
    };
}

inline v3 to_v3(v2 val) {
    return v3 {val.x, val.y, 1.0f};
}

inline v2 to_v2(v3 val) {
    return v2 {val.x, val.y};
}

inline v2 triple(v2 a, v2 b, v2 c) {
    f32 w = b.x * c.y - b.y * c.x;
    return v2 { a.y * w, -a.x * w };
}

inline m3x3 get_translation_matrix(v2 val) {
    m3x3 result = {{0}};
    result.r1 = {1,0,val.x};
    result.r2 = {0,1,val.y};
    result.r3 = {0,0,1};
    return result;
}

inline m3x3 get_rotation_matrix_3x3(f32 theta) {
    m3x3 result;
    result.r1 = {cos(theta), -sin(theta), 0.0f};
    result.r2 = {sin(theta), cos(theta), 0.0f};
    result.r3 = {0.0f, 0.0f, 1.0f};
}

inline v2 operator* (m3x3 lhs, v2 rhs) {
    v2 result;
    result.x = lhs.r1.c1 * rhs.x + lhs.r1.c2 * rhs.y + lhs.r1.c3;
    result.y = lhs.r2.c1 * rhs.x + lhs.r2.c2 * rhs.y + lhs.r2.c3;
    return result;
}

inline m3x3 identity_3x3(){
    m3x3 result;
    result.r1 = {1,0,0};
    result.r2 = {0,1,0};
    result.r3 = {0,0,1};
    return result;
}

inline m3x3 operator*(f32 lhs, m3x3 rhs) {
    m3x3 result = rhs;
    result.r1.c1 *= lhs;
    result.r1.c2 *= lhs;
    result.r1.c3 *= lhs;
    result.r2.c1 *= lhs;
    result.r2.c2 *= lhs;
    result.r2.c3 *= lhs;
    result.r3.c1 *= lhs;
    result.r3.c2 *= lhs;
    result.r3.c3 *= lhs;
    return result;
}

inline m3x3 get_scaling_matrix(f32 ratio) {
    m3x3 result = {0};
    result.r1.c1 = ratio;
    result.r2.c2 = ratio;
    result.r3.c3 = 1.0f;
    return result;
}

inline v6 operator*(f32 lhs, v6 rhs) {
    return v6 {
        lhs * rhs.vals[0],
        lhs * rhs.vals[1],
        lhs * rhs.vals[2],
        lhs * rhs.vals[3],
        lhs * rhs.vals[4],
        lhs * rhs.vals[5]
    };
}

inline f32 dot(v6 lhs, v6 rhs) {
    __m128 l0 = _mm_loadu_ps((const float*)&lhs.vals);
    __m128 r0 = _mm_loadu_ps((const float*)&rhs.vals);
    __m128 l1 = _mm_set_ps(lhs.vals[4], lhs.vals[5], 0.0f, 0.0f);
    __m128 r1 = _mm_set_ps(rhs.vals[4], rhs.vals[5], 0.0f, 0.0f);
    __m128 result =_mm_add_ps(_mm_mul_ps(l0, r0), _mm_mul_ps(l1, r1));
    return result[0] + result[1] + result[2] + result[3];
}

inline v3 to_rgb(u32 color) {
    v3 result;
    result.r = (f32)((color & 0x00ff0000) >> 16) / 255.0f;
    result.g = (f32)((color & 0x0000ff00) >> 8) / 255.0f;
    result.b = (f32)((color & 0x000000ff)) / 255.0f;
    return result;
}

inline u32 from_rgb(v3 color) {
    return  ((0xff000000) |
             ((uround(color.r * 255.0f) << 16) & 0x00ff0000) |
             ((uround(color.g * 255.0f) << 8) & 0x0000ff00) |
             ((uround(color.b * 255.0f) << 0) & 0x000000ff));
}

inline __m128 wide_max(__m128 lhs, __m128 rhs) {
    __m128 cmp = _mm_cmpgt_ps(lhs, rhs);
    return _mm_add_ps(_mm_and_ps(cmp, lhs),_mm_andnot_ps(cmp, rhs));
}

inline __m128 wide_min(__m128 lhs, __m128 rhs) {
    __m128 cmp = _mm_cmplt_ps(lhs, rhs);
    return _mm_add_ps(_mm_and_ps(cmp, lhs),_mm_andnot_ps(cmp, rhs));
}

inline __m128 wide_clamp(__m128 val, __m128 min, __m128 max) {
    return wide_max(wide_min(val, max), min);
}

#endif //PHYSICA_PHYSICA_MATH_H

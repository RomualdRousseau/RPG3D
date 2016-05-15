/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      math3d.h
 *
 *      Copyright 2008 Romuald Rousseau <romualdrousseau@msn.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#ifndef __MATH3D_H__
#define __MATH3D_H__

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>
#include <math.h>
#include <glib.h>

#define EPSILON 0.0001f
#define DEG2RAD (M_PI / 180.0f)
#define RAD2DEG (180.0f / M_PI)

#ifdef FASTMATH_SUPPORT
# define _INV_SQRT(x)       (fast_inv_sqrt(x))
# define _SQRT(x)           (fast_sqrt(x))
# define _ABS(x)            (fabs(x))
#else
# define _INV_SQRT(x)       (1.0f / sqrt(x))
# define _SQRT(x)           (sqrt(x))
# define _ABS(x)            (fabs(x))
#endif
# define _SGN(x)            ((x < -EPSILON) ? -1 : ((x > EPSILON) ? 1 : 0))
# define _SQR(x)            ((x) * (x))

/* --- structures --- */
typedef struct
{
    int     a;
    int     b;
    int     c;
}
int3;

typedef struct
{
    int     a;
    int     b;
    int     c;
    int     d;
}
int4;

typedef struct
{
    float   x;
    float   y;
}
float2;

typedef struct
{
    float   x;
    float   y;
    float   z;
}
float3;

typedef struct
{
    float   x;
    float   y;
    float   z;
    float   w;
}
float4;

typedef struct
{
    float   m00;
    float   m01;
    float   m02;
    float   m03;
    float   m10;
    float   m11;
    float   m12;
    float   m13;
    float   m20;
    float   m21;
    float   m22;
    float   m23;
    float   m30;
    float   m31;
    float   m32;
    float   m33;
}
float4x4;
    
/* --- functions --- */
/*
 * fast_inv_sqrt:
 * @x: a float
 *
 * Returns the Fast Inverse Square Root: Newton Square Root Approximation with
 * a clever (magical) initial guess and one iteration.
 * This code is used by the quake 3 engine but the author is
 * probably Greg Walsh. This code was optimized by Charles McEniry.
 */
static inline float
fast_inv_sqrt(
    float   x
    )
{
    union { float f; uint32_t ul; } y;
    y.f = x;
    y.ul = (0xbe6eb50c - y.ul) >> 1;
    y.f = 0.5f * y.f * ( 3.0f - x * y.f * y.f );
    return y.f;
}

/*
 * fast_sqrt:
 * @x: a float
 *
 * Returns the square root using the Fast Inverse Root Square
 */
static inline float
fast_sqrt(
    float x
    )
{
    return x * fast_inv_sqrt(x);
}

/*
 * fast_fabs:
 * @x: a float
 *
 * Returns the Fast Absolute Value
 */
static inline float
fast_fabs(
    float x
    )
{
    *((int32_t*) &x) &= 0x7fffffff;
    return x;
}

/*
 * length2:
 * @u: a vector
 *
 * Returns the length of a vector
 */
static inline float
length2(
    float2*     u
    )
{
    return _SQRT(_SQR(u->x) + _SQR(u->y));
}

/*
 * length3:
 * @u: a vector
 *
 * Returns the length of a vector
 */
static inline float
length3(
    float3*     u
    )
{
    return _SQRT(_SQR(u->x) + _SQR(u->y) + _SQR(u->z));
}

/*
 * length4:
 * @u: a vector
 *
 * Returns the length of a vector
 */
static inline float
length4(
    float4*     u
    )
{
    return _SQRT(_SQR(u->x) + _SQR(u->y) + _SQR(u->z) + _SQR(u->w));
}

/*
 * Returns the square length of a vector
 */
static inline float
length_sqr2(
    float2*     u
    )
{
    return _SQR(u->x) + _SQR(u->y);
}

/*
 * Returns the square length of a vector
 */
static inline float
length_sqr3(
    float3*     u
    )
{
    return _SQR(u->x) + _SQR(u->y) + _SQR(u->z);
}

/*
 * Returns the square length of a vector
 */
static inline float
length_sqr4(
    float4*     u
    )
{
    return _SQR(u->x) + _SQR(u->y) + _SQR(u->z) + _SQR(u->w);
}

/*
 * Normalizes a vector u.
 */
static inline float2*
norm2(
    float2*     u
    )
{
    float l;

    if(u->x == 0.0f && u->y == 0.0f)
    {
        return u;
    }
    l = _INV_SQRT(_SQR(u->x) + _SQR(u->y));
    u->x *= l;
    u->y *= l;
    return u;
}

/*
 * Normalizes a vector u.
 */
static inline float3*
norm3(
    float3*     u
    )
{
    float l;

    if(u->x == 0.0f && u->y == 0.0f && u->z == 0.0f)
    {
        return u;
    }
    l = _INV_SQRT(_SQR(u->x) + _SQR(u->y) + _SQR(u->z));
    u->x *= l;
    u->y *= l;
    u->z *= l;
    return u;
}

/*
 * Normalizes a vector u.
 */
static inline float4*
norm4(
    float4*     u
    )
{
    float       l;

    if(u->x == 0.0f && u->y == 0.0f && u->z == 0.0f && u->w == 0.0f)
    {
        return u;
    }
    l = _INV_SQRT(_SQR(u->x) + _SQR(u->y) + _SQR(u->z) + _SQR(u->w));
    u->x *= l;
    u->y *= l;
    u->z *= l;
    u->w *= l;
    return u;
}

/*
 * Returns the dot product of u and v.
 */
static inline float
dot2(
    float2*     u,
    float2*     v
    )
{
    return u->x * v->x + u->y * v->y;
}

/*
 * Returns the dot product of u and v.
 */
static inline float
dot3(
    float3*     u,
    float3*     v
    )
{
    return u->x * v->x + u->y * v->y + u->z * v->z;
}

/*
 * Returns the dot product of u and v.
 */
static inline float
dot4(
    float4*     u,
    float4*     v
    )
{
    return u->x * v->x + u->y * v->y + u->z * v->z + u->w * v->w;
}

/*
 * Returns the cross product of u and v.
 */
static inline float2*
cross2(
    float2*     u,
    float2*     v,
    float2*     r
    )
{
    r->x = u->y - v->y;
    r->y = v->x - u->x;
    return r;
}

/*
 * Returns the cross product of u and v.
 */
static inline float3*
cross3(
    float3*     u,
    float3*     v,
    float3*     r
    )
{
    r->x = u->y * v->z - u->z * v->y;
    r->y = u->z * v->x - u->x * v->z;
    r->z = u->x * v->y - u->y * v->x;
    return r;
}

/*
 * Returns the cross product of u and v.
 */
static inline float4*
cross4(
    float4*     u,
    float4*     v,
    float4*     r
    )
{
    r->x = u->y * v->z - u->z * v->y;
    r->y = u->z * v->w - u->w * v->z;
    r->z = u->w * v->x - u->x * v->w;
    r->w = u->x * v->y - u->y * v->x;
    return r;
}

/*
 * Multiplies the vector u by the matrix m.
 */
static inline float2*
mul2(
    float2*     u,
    float4x4*   m,
    float2*     r
    )
{
    r->x = u->x * m->m00 + u->y * m->m10 + m->m20 + m->m30;
    r->y = u->x * m->m01 + u->y * m->m11 + m->m21 + m->m31;
    return r;
}

/*
 * Multiplies the vector u by the matrix m.
 */
static inline float3*
mul3(
    float3*     u,
    float4x4*   m,
    float3*     r
    )
{
    r->x = u->x * m->m00 + u->y * m->m10 + u->z * m->m20 + m->m30;
    r->y = u->x * m->m01 + u->y * m->m11 + u->z * m->m21 + m->m31;
    r->z = u->x * m->m02 + u->y * m->m12 + u->z * m->m22 + m->m32;
    return r;
}

/*
 * Transforms the point p by the matrix m.
 */
static inline float4*
mul4(
    float4*     u,
    float4x4*   m,
    float4*     r
    )
{
    r->x = u->x * m->m00 + u->y * m->m10 + u->z * m->m20 + u->w * m->m30;
    r->y = u->x * m->m01 + u->y * m->m11 + u->z * m->m21 + u->w * m->m31;
    r->z = u->x * m->m02 + u->y * m->m12 + u->z * m->m22 + u->w * m->m32;
    r->w = u->x * m->m03 + u->y * m->m13 + u->z * m->m23 + u->w * m->m33;
    return r;
}


/*
 * Multiply 2 matrices.
 */
static inline float4x4*
mul4x4(
    float4x4*   a,
    float4x4*   b,
    float4x4*   r
    )
{
    float bi0, bi1, bi2, bi3;
    
    bi0 = b->m00; bi1 = b->m10; bi2 = b->m20; bi3 = b->m30; 
    r->m00 = bi0 * a->m00 + bi1 * a->m01 + bi2 * a->m02 + bi3 * a->m03;
    r->m10 = bi0 * a->m10 + bi1 * a->m11 + bi2 * a->m12 + bi3 * a->m13;
    r->m20 = bi0 * a->m20 + bi1 * a->m21 + bi2 * a->m22 + bi3 * a->m23;
    r->m30 = bi0 * a->m30 + bi1 * a->m31 + bi2 * a->m32 + bi3 * a->m33;
    bi0 = b->m01; bi1 = b->m11; bi2 = b->m21; bi3 = b->m31;
    r->m01 = bi0 * a->m00 + bi1 * a->m01 + bi2 * a->m02 + bi3 * a->m03;
    r->m11 = bi0 * a->m10 + bi1 * a->m11 + bi2 * a->m12 + bi3 * a->m13;
    r->m21 = bi0 * a->m20 + bi1 * a->m21 + bi2 * a->m22 + bi3 * a->m23;
    r->m31 = bi0 * a->m30 + bi1 * a->m31 + bi2 * a->m32 + bi3 * a->m33;
    bi0 = b->m02; bi1 = b->m12; bi2 = b->m22; bi3 = b->m32;
    r->m02 = bi0 * a->m00 + bi1 * a->m01 + bi2 * a->m02 + bi3 * a->m03;
    r->m12 = bi0 * a->m10 + bi1 * a->m11 + bi2 * a->m12 + bi3 * a->m13;
    r->m22 = bi0 * a->m20 + bi1 * a->m21 + bi2 * a->m22 + bi3 * a->m23;
    r->m32 = bi0 * a->m30 + bi1 * a->m31 + bi2 * a->m32 + bi3 * a->m33;
    bi0 = b->m03; bi1 = b->m13; bi2 = b->m23; bi3 = b->m33;
    r->m03 = bi0 * a->m00 + bi1 * a->m01 + bi2 * a->m02 + bi3 * a->m03;
    r->m13 = bi0 * a->m10 + bi1 * a->m11 + bi2 * a->m12 + bi3 * a->m13;
    r->m23 = bi0 * a->m20 + bi1 * a->m21 + bi2 * a->m22 + bi3 * a->m23;
    r->m33 = bi0 * a->m30 + bi1 * a->m31 + bi2 * a->m32 + bi3 * a->m33;
    return r;
}

/*
 * Linear Interpolation between two point.
 */
static inline float
lerp(
    float       a,
    float       b,
    float       t
    )
{
    float _t = 1.0f - t;
    return b * t + a * _t;
}

/*
 * Linear Interpolation between two point.
 */
static inline float2*
lerp2(
    float2*     a,
    float2*     b,
    float       t,
    float2*     r
    )
{
    float _t = 1.0f - t;
    r->x = b->x * t + a->x * _t;
    r->y = b->y * t + a->y * _t;
    return r;
}

/*
 * Linear Interpolation between two point.
 */
static inline float3*
lerp3(
    float3* a,
    float3* b,
    float   t,
    float3* r
    )
{
    float _t = 1.0f - t;
    r->x = b->x * t + a->x * _t;
    r->y = b->y * t + a->y * _t;
    r->z = b->z * t + a->z * _t;
    return r;
}

/*
 * Linear Interpolation between two point.
 */
static inline float4*
lerp4(
    float4* a,
    float4* b,
    float   t,
    float4* r
    )
{
    float _t = 1.0f - t;
    r->x = b->x * t + a->x * _t;
    r->y = b->y * t + a->y * _t;
    r->z = b->z * t + a->z * _t;
    r->w = b->w * t + a->w * _t;
    return r;
}

/* Matrices */

static inline float4x4*
r_matrix_identity_set(
    float4x4*   m
    )
{
    m->m00 = 1.0f; m->m01 = 0.0f; m->m02 = 0.0f; m->m03 = 0.0f;
    m->m10 = 0.0f; m->m11 = 1.0f; m->m12 = 0.0f; m->m13 = 0.0f;
    m->m20 = 0.0f; m->m21 = 0.0f; m->m22 = 1.0f; m->m23 = 0.0f;
    m->m30 = 0.0f; m->m31 = 0.0f; m->m32 = 0.0f; m->m33 = 1.0f;
    return m;
}

extern float4x4*
r_matrix_frustum_set(
    float4x4*   m,
    float       left,
    float       right,
    float       bottom,
    float       top,
    float       znear,
    float       zfar
    );

extern float4x4*
r_matrix_ortho_set(
    float4x4*   m,
    float       left,
    float       right,
    float       bottom,
    float       top,
    float       znear,
    float       zfar
    );

extern float4x4*
r_matrix_translate(
    float4x4*   m,
    float3*     v
    );

extern float4x4*
r_matrix_rotate(
    float4x4*   m,
    float       angle,
    float3*     axis
    );

extern void
r_matrix_print(
    float4x4*   m
    );

/* Frustum */

extern void
r_frustum_update_projection(
    float4x4*   projection
    );

extern gboolean
r_frustum_test_point(
    float4x4*   view,
    float3*     point
    );
    
extern gboolean
r_frustum_test_bsphere(
    float4x4*   view,
    float3*     bshere
    );
    
extern gboolean
r_frustum_test_bbox(
    float4x4*   view,
    float3*     bbox
    );

/* Collision */

extern gboolean
r_triangle_contain_point(
    float3*     triangle,
    float3*     point_to_test
    );

extern void
r_triangle_to_plane(
    float3*     triangle,
    float4*     plane
    );

extern float3*
r_bbox_translate(
    float3*     bbox,
    float3*     v,
    float3*     r
    );

extern gboolean
r_bbox_overlap(
    float3*     bbox1,
    float3*     bbox2
    );

#endif /* __MATH3D_H__ */

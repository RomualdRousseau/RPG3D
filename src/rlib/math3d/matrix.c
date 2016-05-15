/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * matrix.c
 * 
 * Copyright 2012 Romuald Rousseau <romuald@you-are-a-wizard>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <rlib.h>

/* --- functions --- */
/**
 * matrix_frustum_set:
 *
 **/
float4x4*
r_matrix_frustum_set(
    float4x4*   matrix,
    float       left,
    float       right,
    float       bottom,
    float       top,
    float       znear,
    float       zfar
    )
{
    const float a = (2.0f * znear) / (right - left);
    const float b = (2.0f * znear) / (top - bottom);
    const float c = (right + left) / (right - left);
    const float d = (top + bottom) / (top - bottom);
    const float e = -(zfar + znear) / ( zfar - znear);
    const float f = -(2.0f * zfar * znear) / (zfar - znear);
    matrix->m00 = a;    matrix->m01 = 0.0f; matrix->m02 = c; matrix->m03 = 0.0f;
    matrix->m10 = 0.0f; matrix->m11 = b;    matrix->m12 = d; matrix->m13 = 0.0f;
    matrix->m20 = 0.0f; matrix->m21 = 0.0f; matrix->m22 = e; matrix->m23 = -1.0f;
    matrix->m30 = 0.0f; matrix->m31 = 0.0f; matrix->m32 = f; matrix->m33 = 0.0f;
    return matrix;
}

/**
 * matrix_ortho_set:
 *
 **/
float4x4*
r_matrix_ortho_set(
    float4x4*   matrix,
    float       left,
    float       right,
    float       bottom,
    float       top,
    float       znear,
    float       zfar
    )
{
    const float a = 2.0F / (right - left);
    const float b = -(right + left) / (right - left);
    const float c = 2.0F / (top - bottom);
    const float d = -(top + bottom) / (top - bottom);
    const float e = -2.0F / (zfar - znear);
    const float f = -(zfar + znear) / (zfar - znear);
    matrix->m00 = a;    matrix->m01 = 0.0f; matrix->m02 = 0.0f; matrix->m03 = 0.0f;
    matrix->m10 = 0.0f; matrix->m11 = c;    matrix->m12 = 0.0f; matrix->m13 = 0.0f;
    matrix->m20 = 0.0f; matrix->m21 = 0.0f; matrix->m22 = e;    matrix->m23 = 0.0f;
    matrix->m30 = b;    matrix->m31 = d;    matrix->m32 = f;    matrix->m33 = 1.0F;
    return matrix;
}

/**
 * matrix_translate:
 *
 **/
float4x4*
r_matrix_translate(
    float4x4*   matrix,
    float3*     vector
    )
{
    const float x = vector->x;
    const float y = vector->y;
    const float z = vector->z;
    matrix->m30 += matrix->m00 * x + matrix->m10 * y + matrix->m20 * z;
    matrix->m31 += matrix->m01 * x + matrix->m11 * y + matrix->m21 * z;
    matrix->m32 += matrix->m02 * x + matrix->m12 * y + matrix->m22 * z;
    matrix->m33 += matrix->m03 * x + matrix->m13 * y + matrix->m23 * z;
    return matrix;
}

/**
 * matrix_rotate:
 *
 **/
float4x4*
r_matrix_rotate(
    float4x4*   matrix,
    float       angle,
    float3*     axis
    )
{
    float x, y, z, xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c, s, c;
    float4x4 m;
    gboolean optimized;

    x = axis->x;
    y = axis->y;
    z = axis->z;
    s = sin(angle * DEG2RAD);
    c = cos(angle * DEG2RAD);

    r_matrix_identity_set(&m);
    optimized = FALSE;

    if(x == 0.0f)
    {
        if(y == 0.0f)
        {
            if(z != 0.0f)
            {
                /* rotate only around z-axis */
                optimized = TRUE;
                m.m00 = c;
                m.m11 = c;
                if(z < 0.0f)
                {
                   m.m10 = s;
                   m.m01 = -s;
                }
                else
                {
                   m.m10 = -s;
                   m.m01 = s;
                }
            }
        }
        else if(z == 0.0f) 
        {
            /* rotate only around y-axis */
            optimized = TRUE;
            m.m00 = c;
            m.m22 = c;
            if(y < 0.0f)
            {
               m.m20 = -s;
               m.m02 = s;
            }
            else
            {
               m.m20 = s;
               m.m02 = -s;
            }
        }
    }
    else if(y == 0.0f)
    {
        if(z == 0.0F)
        {
            /* rotate only around x-axis */
            optimized = GL_TRUE;
            m.m11 = c;
            m.m22 = c;
            if(x < 0.0f)
            {
               m.m21 = s;
               m.m12 = -s;
            }
            else
            {
               m.m21 = -s;
               m.m12 = s;
            }
        }
    }

    if (!optimized)
    {
        const float mag = length3(axis);

        if (mag <= EPSILON)
        {
            /* no rotation, leave mat as-is */
            return matrix;
        }

        x /= mag;
        y /= mag;
        z /= mag;

        /*
        *     Arbitrary axis rotation matrix.
        *
        *  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
        *  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
        *  (which is about the X-axis), and the two composite transforms
        *  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
        *  from the arbitrary axis to the X-axis then back.  They are
        *  all elementary rotations.
        *
        *  Rz' is a rotation about the Z-axis, to bring the axis vector
        *  into the x-z plane.  Then Ry' is applied, rotating about the
        *  Y-axis to bring the axis vector parallel with the X-axis.  The
        *  rotation about the X-axis is then performed.  Ry and Rz are
        *  simply the respective inverse transforms to bring the arbitrary
        *  axis back to it's original orientation.  The first transforms
        *  Rz' and Ry' are considered inverses, since the data from the
        *  arbitrary axis gives you info on how to get to it, not how
        *  to get away from it, and an inverse must be applied.
        *
        *  The basic calculation used is to recognize that the arbitrary
        *  axis vector (x, y, z), since it is of unit length, actually
        *  represents the sines and cosines of the angles to rotate the
        *  X-axis to the same orientation, with theta being the angle about
        *  Z and phi the angle about Y (in the order described above)
        *  as follows:
        *
        *  cos ( theta ) = x / sqrt ( 1 - z^2 )
        *  sin ( theta ) = y / sqrt ( 1 - z^2 )
        *
        *  cos ( phi ) = sqrt ( 1 - z^2 )
        *  sin ( phi ) = z
        *
        *  Note that cos ( phi ) can further be inserted to the above
        *  formulas:
        *
        *  cos ( theta ) = x / cos ( phi )
        *  sin ( theta ) = y / sin ( phi )
        *
        *  ...etc.  Because of those relations and the standard trigonometric
        *  relations, it is possible to reduce the transforms down to what
        *  is used below. It may be that any primary axis chosen will give the
        *  same results (modulo a sign convention) using this method.
        *
        *  Particularly nice is to notice that all divisions that might
        *  have caused trouble when parallel to certain planes or
        *  axis go away with care paid to reducing the expressions.
        *  After checking, it does perform correctly under all cases, since
        *  in all the cases of division where the denominator would have
        *  been zero, the numerator would have been zero as well, giving
        *  the expected result.
        */

        xx = x * x;
        yy = y * y;
        zz = z * z;
        xy = x * y;
        yz = y * z;
        zx = z * x;
        xs = x * s;
        ys = y * s;
        zs = z * s;
        one_c = 1.0f - c;

        m.m00 = (one_c * xx) + c;  m.m01 = (one_c * xy) + zs; m.m02 = (one_c * zx) - ys;
        m.m10 = (one_c * xy) - zs; m.m11 = (one_c * yy) + c;  m.m12 = (one_c * yz) + xs;
        m.m20 = (one_c * zx) + ys; m.m21 = (one_c * yz) - xs; m.m22 = (one_c * zz) + c;
    }

    return mul4x4(&m, matrix, matrix);
}

/**
 * matrix_print:
 *
 **/
void
r_matrix_print(
    float4x4*   matrix
    )
{
    g_message("| %f\t%f\t%f\t%f |", matrix->m00, matrix->m01, matrix->m02, matrix->m03);
    g_message("| %f\t%f\t%f\t%f |", matrix->m10, matrix->m11, matrix->m12, matrix->m13);
    g_message("| %f\t%f\t%f\t%f |", matrix->m20, matrix->m21, matrix->m22, matrix->m23);
    g_message("| %f\t%f\t%f\t%f |", matrix->m30, matrix->m31, matrix->m32, matrix->m33);
    g_message(" ");
}

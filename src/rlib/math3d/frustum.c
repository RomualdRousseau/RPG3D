/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      math3d.c
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

#include <rlib.h>

/* --- variables --- */
static float frustum[6][4];

/* --- functions --- */
/**
 * frustum_update_projection:
 *
 **/
void
r_frustum_update_projection(
    float4x4*   projection
    )
{
    frustum[0][0] = projection->m03 - projection->m00;
    frustum[0][1] = projection->m13 - projection->m10;
    frustum[0][2] = projection->m23 - projection->m20;
    frustum[0][3] = projection->m33 - projection->m30;
    
    frustum[1][0] = projection->m03 + projection->m00;
    frustum[1][1] = projection->m13 + projection->m10;
    frustum[1][2] = projection->m23 + projection->m20;
    frustum[1][3] = projection->m33 + projection->m30;
    
    frustum[2][0] = projection->m03 + projection->m01;
    frustum[2][1] = projection->m13 + projection->m11;
    frustum[2][2] = projection->m23 + projection->m21;
    frustum[2][3] = projection->m33 + projection->m31;
    
    frustum[3][0] = projection->m03 - projection->m01;
    frustum[3][1] = projection->m13 - projection->m11;
    frustum[3][2] = projection->m23 - projection->m21;
    frustum[3][3] = projection->m33 - projection->m31;
    
    frustum[4][0] = projection->m03 - projection->m02;
    frustum[4][1] = projection->m13 - projection->m12;
    frustum[4][2] = projection->m23 - projection->m22;
    frustum[4][3] = projection->m33 - projection->m32;
    
    frustum[4][0] = projection->m03 + projection->m02;
    frustum[5][1] = projection->m13 + projection->m12;
    frustum[5][2] = projection->m23 + projection->m22;
    frustum[5][3] = projection->m33 + projection->m32;
}

/**
 * r_frustum_test_point:
 *
 **/
gboolean
r_frustum_test_point(
    float4x4*   view,
    float3*     point
    )
{
    // Not implemented
    return FALSE;
}

/**
 * r_frustum_test_bsphere:
 *
 **/
gboolean
r_frustum_test_bsphere(
    float4x4*   view,
    float3*     bshere
    )
{
    // Not implemented
    return FALSE;
}

/**
 * frustum_test_bbox:
 *
 **/
gboolean
r_frustum_test_bbox(
    float4x4*   view,
    float3*     bbox
    )
{
    float3 p;
    float3 rbbox[8];
    int i, j;
    int r = 0;
    float* plane;
    
    p.x = bbox[0].x - bbox[1].x;
    p.y = bbox[0].y + bbox[1].y;
    p.z = bbox[0].z + bbox[1].z;
    mul3(&p, view, &rbbox[0]);
    p.x = bbox[0].x + bbox[1].x;
    p.y = bbox[0].y + bbox[1].y;
    p.z = bbox[0].z + bbox[1].z;
    mul3(&p, view, &rbbox[1]);
    p.x = bbox[0].x + bbox[1].x;
    p.y = bbox[0].y - bbox[1].y;
    p.z = bbox[0].z + bbox[1].z;
    mul3(&p, view, &rbbox[2]);
    p.x = bbox[0].x - bbox[1].x;
    p.y = bbox[0].y - bbox[1].y;
    p.z = bbox[0].z + bbox[1].z;
    mul3(&p, view, &rbbox[3]);
    p.x = bbox[0].x - bbox[1].x;
    p.y = bbox[0].y + bbox[1].y;
    p.z = bbox[0].z - bbox[1].z;
    mul3(&p, view, &rbbox[4]);
    p.x = bbox[0].x + bbox[1].x;
    p.y = bbox[0].y + bbox[1].y;
    p.z = bbox[0].z - bbox[1].z;
    mul3(&p, view, &rbbox[5]);
    p.x = bbox[0].x + bbox[1].x;
    p.y = bbox[0].y - bbox[1].y;
    p.z = bbox[0].z - bbox[1].z;
    mul3(&p, view, &rbbox[6]);
    p.x = bbox[0].x - bbox[1].x;
    p.y = bbox[0].y - bbox[1].y;
    p.z = bbox[0].z - bbox[1].z;
    mul3(&p, view, &rbbox[7]);

    for(i = 0; i < 6; i++)
    {
        plane = frustum[i];
        for(j = 0; j < 8; j++)
        {
            if(plane[0] * rbbox[j].x + plane[1] * rbbox[j].y + plane[2] * rbbox[j].z + plane[3] > 0.0f)
            {
                r |= (1 << i);
                break;
            }
        }
    }
    
    return ((r & 0x3F) == 0x3F) ? TRUE : FALSE;
}

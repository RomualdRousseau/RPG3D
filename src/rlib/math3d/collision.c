/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      collision.c
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

gboolean
r_triangle_contain_point(
    float3* triangle,
    float3* point_to_test
    )
{
    float3 v0;
    float3 v1;
    float3 v2;
    float dot00;
    float dot01;
    float dot02;
    float dot11;
    float dot12;
    float k;
    float u;
    float v;

    v0.x = triangle[2].x - triangle[0].x;
    v0.y = triangle[2].y - triangle[0].y;
    v0.z = triangle[2].z - triangle[0].z;

    v1.x = triangle[1].x - triangle[0].x;
    v1.y = triangle[1].y - triangle[0].y;
    v1.z = triangle[1].z - triangle[0].z;

    v2.x = point_to_test->x - triangle[0].x;
    v2.y = point_to_test->y - triangle[0].y;
    v2.z = point_to_test->z - triangle[0].z;

    dot00 = dot3(&v0, &v0);
    dot01 = dot3(&v0, &v1);
    dot02 = dot3(&v0, &v2);
    dot11 = dot3(&v1, &v1);
    dot12 = dot3(&v1, &v2);

    k = 1.0f / (dot00 * dot11 - dot01 * dot01);
    u = (dot11 * dot02 - dot01 * dot12) * k;
    v = (dot00 * dot12 - dot01 * dot02) * k;

    return ((u >= 0.0f) && (v >= 0.0f) && (u + v < 1.0f)) ? TRUE : FALSE;
}

void
r_triangle_to_plane(
    float3* triangle,
    float4* plane
    )
{
    float3 v0;
    float3 v1;

    v0.x = triangle[2].x - triangle[0].x;
    v0.y = triangle[2].y - triangle[0].y;
    v0.z = triangle[2].z - triangle[0].z;

    v1.x = triangle[1].x - triangle[0].x;
    v1.y = triangle[1].y - triangle[0].y;
    v1.z = triangle[1].z - triangle[0].z;

    plane->w = -dot3(&triangle[0], norm3(cross3(&v1, &v0, (float3*) plane)));
}

float3*
r_bbox_translate(
    float3*     bbox,
    float3*     v,
    float3*     r
    )
{
    r[0].x = bbox[0].x + v->x;
    r[0].y = bbox[0].y + v->y;
    r[0].z = bbox[0].z + v->z;
    r[1].x = bbox[1].x;
    r[1].y = bbox[1].y;
    r[1].z = bbox[1].z;
    return r;
}

gboolean
r_bbox_overlap(
    float3* bbox1,
    float3* bbox2
    )
{
    return ( 
           (bbox2[0].x + bbox2[1].x) >= (bbox1[0].x - bbox1[1].x) && (bbox2[0].x - bbox2[1].x) <= (bbox1[0].x + bbox1[1].x)
        && (bbox2[0].y + bbox2[1].y) >= (bbox1[0].y - bbox1[1].y) && (bbox2[0].y - bbox2[1].y) <= (bbox1[0].y + bbox1[1].y)
        && (bbox2[0].z + bbox2[1].z) >= (bbox1[0].z - bbox1[1].z) && (bbox2[0].z - bbox2[1].z) <= (bbox1[0].z + bbox1[1].z)
        ) ? TRUE : FALSE;
}

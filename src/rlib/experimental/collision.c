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

static int
_bbox_get_voronoi_space(
    float3* point,
    float3* bmin,
    float3* bmax
    )
{
    int result = 0;

    if(point->x >= bmax->x)
    {
        result |= 1;
    }
    else if(point->x <= bmin->x)
    {
        result |= 2;
    }
    if(point->y >= bmax->y)
    {
        result |= 4;
    }
    else if(point->y <= bmin->y)
    {
        result |= 8;
    }
    if(point->z >= bmax->z)
    {
        result |= 16;
    }
    else if(point->z <= bmin->z)
    {
        result |= 32;
    }
    return result;
}

static int
_bsphere_test_point(
    float3*     point,
    float3*     bsphere,
    float3*     reaction
    )
{
    float ry = bsphere[1].x / bsphere[1].y;
    float rz = bsphere[1].x / bsphere[1].z;
    float d;
    float t;

    reaction->x = (bsphere[0].x - point->x);
    reaction->y = (bsphere[0].y - point->y) * ry;
    reaction->z = (bsphere[0].z - point->z) * rz;

    d = length_sqr3(reaction);

    if(d > _SQR(bsphere[1].x))
    {
        return 0;
    }

    if(d != 0.0f)
    {
        d = _SQRT(d);
        t = (bsphere[1].x - d) / d;
        reaction->x *= t;
        reaction->y *= t / ry;
        reaction->z *= t / rz;
    }

    return 1;
}

static int
_bsphere_test_edge(
    float3*     edge,
    float3*     bshpere,
    float3*     reaction
    )
{
    float3 p;
    line_closest_point(edge, &bshpere[0], &p);
    return _bsphere_test_point(&p, bshpere, reaction);
}

static int
_bsphere_test_plane(
    float3*     plane,
    float3*     bsphere,
    float3*     reaction
    )
{
    float ry = bsphere[1].x / bsphere[1].y;
    float rz = bsphere[1].x / bsphere[1].z;
    float d;
    float t;

    reaction->x = (bsphere[0].x - plane[0].x) * plane[1].x;
    reaction->y = (bsphere[0].y - plane[0].y) * plane[1].y * ry;
    reaction->z = (bsphere[0].z - plane[0].z) * plane[1].z * rz;

    d = _ABS(reaction->x + reaction->y + reaction->z);

    if(d > bsphere[1].x)
    {
        return 0;
    }

    if(d != 0.0f)
    {
        t = (bsphere[1].x - d) / d;
        reaction->x *= t;
        reaction->y *= t / ry;
        reaction->z *= t / rz;
    }

    return 1;
}

int
triangle_contain_point(
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

    return (u >= 0.0f) && (v >= 0.0f) && (u + v < 1.0f);
}

void
triangle_to_plane(
    float3* triangle,
    float3* plane
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

    plane[0].x = triangle[0].x;
    plane[0].y = triangle[0].y;
    plane[0].z = triangle[0].z;

    norm3(cross3(&v0, &v1, &plane[1]));
}

int
plane_intersect_line(
    float3* plane,
    float3* edge_start,
    float3* edge_stop,
    float*  returned_ratio
    )
{
    float3 ab;
    float3 ao;
    float k;

    ab.x = edge_stop->x - edge_start->x;
    ab.y = edge_stop->y - edge_start->y;
    ab.z = edge_stop->z - edge_start->z;

    k = dot3(&plane[1], &ab);
    if(k == 0.0f)
    {
        return 0; // collinear => no intersection !
    }

    ao.x = plane[0].x - edge_start->x;
    ao.y = plane[0].y - edge_start->y;
    ao.z = plane[0].z - edge_start->z;

    *returned_ratio = dot3(&plane[1], &ao) / k;

    return 1;
}

int
plane_intersect_segment(
    float3* plane,
    float3* edge_start,
    float3* edge_stop,
    float*  returned_ratio
    )
{
    float3 ab;
    float3 ao;
    float k;

    ab.x = edge_stop->x - edge_start->x;
    ab.y = edge_stop->y - edge_start->y;
    ab.z = edge_stop->z - edge_start->z;

    k = dot3(&plane[1], &ab);
    if(k == 0.0f)
    {
        return 0; // collinear => no intersection !
    }

    ao.x = plane[0].x - edge_start->x;
    ao.y = plane[0].y - edge_start->y;
    ao.z = plane[0].z - edge_start->z;

    k = dot3(&plane[1], &ao) / k;
    if(k < 0.0f || k > 1.0f)
    {
        return 0; // outside of segment => no intersection !
    }

    *returned_ratio = k;

    return 1;
}

float
plane_signed_closest_distance(
    float3* plane,
    float3* point_to_test
    )
{
    float3 op;

    op.x = point_to_test->x - plane[0].x;
    op.y = point_to_test->y - plane[0].y;
    op.z = point_to_test->z - plane[0].z;

    return dot3(&op, &plane[1]);
}

float
plane_signed_closest_point(
    float3* plane,
    float3* point_to_test,
    float3* returned_point
    )
{
    float3 op;
    float k;

    op.x = point_to_test->x - plane[0].x;
    op.y = point_to_test->y - plane[0].y;
    op.z = point_to_test->z - plane[0].z;

    k = dot3(&op, &plane[1]);

    returned_point->x = k * -plane[1].x + point_to_test->x;
    returned_point->y = k * -plane[1].y + point_to_test->y;
    returned_point->z = k * -plane[1].z + point_to_test->z;

    return k;
}

void
line_closest_point(
    float3* line,
    float3* point_to_test,
    float3* returned_point
    )
{
    float3 u;
    float3 v;
    float t;

    u.x = line[1].x - line[0].x;
    u.y = line[1].y - line[0].y;
    u.z = line[1].z - line[0].z;

    v.x = point_to_test->x - line[0].x;
    v.y = point_to_test->y - line[0].y;
    v.z = point_to_test->z - line[0].z;

    t = dot3(&u, &v) / length_sqr3(&u);
    returned_point->x = t * u.x + line[0].x;
    returned_point->y = t * u.y + line[0].y;
    returned_point->z = t * u.z + line[0].z;
}

void
bsphere_closest_point(
    float3* sphere,
    float3* point_to_test,
    float3* returned_point
    )
{
    float3 u;

    u.x = point_to_test->x - sphere[0].x;
    u.y = point_to_test->y - sphere[0].y;
    u.z = point_to_test->z - sphere[0].z;

    norm3(&u);
    returned_point->x = u.x * sphere[1].x + sphere[0].x;
    returned_point->y = u.y * sphere[1].y + sphere[0].y;
    returned_point->z = u.z * sphere[1].z + sphere[0].z;
}

void
bbox_closest_point(
    float3* bbox,
    float3* point_to_test,
    float3* returned_point
    )
{
    float3 bmin = {bbox[0].x - bbox[1].x, bbox[0].y - bbox[1].y, bbox[0].z - bbox[1].z};
    float3 bmax = {bbox[0].x + bbox[1].x, bbox[0].y + bbox[1].y, bbox[0].z + bbox[1].z};
    float3 m[2];

    switch(_bbox_get_voronoi_space(point_to_test, &bmin, &bmax))
    {
    case 0:
        returned_point->x = (point_to_test->x >= bbox[0].x) ? bmax.x : bmin.x;
        returned_point->y = (point_to_test->y >= bbox[0].y) ? bmax.y : bmin.y;
        returned_point->z = (point_to_test->z >= bbox[0].z) ? bmax.z : bmin.z;
        break;
    case 1:
        returned_point->x = bmax.x;
        returned_point->y = point_to_test->y;
        returned_point->z = point_to_test->z;
        break;
    case 2:
        returned_point->x = bmin.x;
        returned_point->y = point_to_test->y;
        returned_point->z = point_to_test->z;
        break;
    case 4:
        returned_point->x = point_to_test->x;
        returned_point->y = bmax.y;
        returned_point->z = point_to_test->z;
        break;
    case 5:
        m[0].x = bmax.x; m[0].y = bmax.y; m[0].z = bmin.z;
        m[1].x = bmax.x; m[1].y = bmax.y; m[1].z = bmax.z;
        line_closest_point(m, point_to_test, returned_point);
        break;
    case 6:
        m[0].x = bmin.x; m[0].y = bmax.y; m[0].z = bmin.z;
        m[1].x = bmin.x; m[1].y = bmax.y; m[1].z = bmax.z;
        line_closest_point(m, point_to_test, returned_point);
        break;
    case 8:
        returned_point->x = point_to_test->x;
        returned_point->y = bmin.y;
        returned_point->z = point_to_test->z;
        break;
    case 9:
        m[0].x = bmax.x; m[0].y = bmin.y; m[0].z = bmin.z;
        m[1].x = bmax.x; m[1].y = bmin.y; m[1].z = bmax.z;
        line_closest_point(m, point_to_test, returned_point);
        break;
    case 10:
        m[0].x = bmin.x; m[0].y = bmin.y; m[0].z = bmin.z;
        m[1].x = bmin.x; m[1].y = bmin.y; m[1].z = bmax.z;
        line_closest_point(m, point_to_test, returned_point);
        break;
    case 16:
        returned_point->x = point_to_test->x;
        returned_point->y = point_to_test->y;
        returned_point->z = bmax.z;
        break;
    case 17:
        m[0].x = bmax.x; m[0].y = bmin.y; m[0].z = bmax.z;
        m[1].x = bmax.x; m[1].y = bmax.y; m[1].z = bmax.z;
        line_closest_point(m, point_to_test, returned_point);
        break;
    case 18:
        m[0].x = bmin.x; m[0].y = bmin.y; m[0].z = bmax.z;
        m[1].x = bmin.x; m[1].y = bmax.y; m[1].z = bmax.z;
        line_closest_point(m, point_to_test, returned_point);
        break;
    case 20:
        m[0].x = bmin.x; m[0].y = bmax.y; m[0].z = bmax.z;
        m[1].x = bmax.x; m[1].y = bmax.y; m[1].z = bmax.z;
        line_closest_point(m, point_to_test, returned_point);
        break;
    case 21:
        returned_point->x = bmax.x;
        returned_point->y = bmax.y;
        returned_point->z = bmax.z;
        break;
    case 22:
        returned_point->x = bmin.x;
        returned_point->y = bmax.y;
        returned_point->z = bmax.z;
        break;
    case 24:
        m[0].x = bmin.x; m[0].y = bmin.y; m[0].z = bmax.z;
        m[1].x = bmax.x; m[1].y = bmin.y; m[1].z = bmax.z;
       line_closest_point(m, point_to_test, returned_point);
        break;
    case 25:
        returned_point->x = bmax.x;
        returned_point->y = bmin.y;
        returned_point->z = bmax.z;
        break;
    case 26:
        returned_point->x = bmin.x;
        returned_point->y = bmin.y;
        returned_point->z = bmax.z;
        break;
    case 32:
        returned_point->x = point_to_test->x;
        returned_point->y = point_to_test->y;
        returned_point->z = bmin.z;
        break;
    case 33:
        m[0].x = bmax.x; m[0].y = bmin.y; m[0].z = bmin.z;
        m[1].x = bmax.x; m[1].y = bmax.y; m[1].z = bmin.z;
        line_closest_point(m, point_to_test, returned_point);
        break;
    case 34:
        m[0].x = bmax.x; m[0].y = bmin.y; m[0].z = bmin.z;
        m[1].x = bmax.x; m[1].y = bmax.y; m[1].z = bmin.z;
        line_closest_point(m, point_to_test, returned_point);
        break;
    case 36:
        m[0].x = bmin.x; m[0].y = bmax.y; m[0].z = bmin.z;
        m[1].x = bmax.x; m[1].y = bmax.y; m[1].z = bmin.z;
        line_closest_point(m, point_to_test, returned_point);
        break;
    case 37:
        returned_point->x = bmax.x;
        returned_point->y = bmax.y;
        returned_point->z = bmin.z;
        break;
    case 38:
        returned_point->x = bmin.x;
        returned_point->y = bmax.y;
        returned_point->z = bmin.z;
        break;
    case 40:
        m[0].x = bmin.x; m[0].y = bmin.y; m[0].z = bmin.z;
        m[1].x = bmax.x; m[1].y = bmin.y; m[1].z = bmin.z;
        line_closest_point(m, point_to_test, returned_point);
        break;
    case 41:
        returned_point->x = bmax.x;
        returned_point->y = bmin.y;
        returned_point->z = bmin.z;
        break;
    case 42:
        returned_point->x = bmin.x;
        returned_point->y = bmin.y;
        returned_point->z = bmin.z;
        break;
    }
}

int
bsphere_bsphere_overlap(
    float3*     bsphere1,
    float3*     bsphere2
    )
{
    float3 u;
    float3 p;

    bsphere_closest_point(bsphere1, &bsphere2[0], &p);
    u.x = (bsphere2[0].x - p.x);
    u.y = (bsphere2[0].y - p.y) * bsphere2[1].x / bsphere2[1].y;
    u.z = (bsphere2[0].z - p.z) * bsphere2[1].x / bsphere2[1].z;
    return (length_sqr3(&u) <= _SQR(bsphere2[1].x)) ? 1 : 0;
}

int
bsphere_bsphere_collide(
    float3*     bsphere1,
    float3*     bsphere2,
    float3*     reaction
    )
{
    float3 p;
    bsphere_closest_point(bsphere1, &bsphere2[0], &p);
    return _bsphere_test_point(&p, bsphere2, reaction);
}

int
bbox_bbox_overlap(
    float3*     bbox1,
    float3*     bbox2
    )
{
    return ((_ABS(bbox1[0].x - bbox2[0].x) <= (bbox1[1].x + bbox2[1].x))
          && (_ABS(bbox1[0].y - bbox2[0].y) <= (bbox1[1].y + bbox2[1].y))
          && (_ABS(bbox1[0].z - bbox2[0].z) <= (bbox1[1].z + bbox2[1].z))
          ) ? 1 : 0;
}

int
bbox_bbox_collide(
    float3*     bbox1,
    float3*     bbox2,
    float3*     reaction
    )
{
    float3 bmin = {bbox1[0].x - bbox1[1].x, bbox1[0].y - bbox1[1].y, bbox1[0].z - bbox1[1].z};
    float3 bmax = {bbox1[0].x + bbox1[1].x, bbox1[0].y + bbox1[1].y, bbox1[0].z + bbox1[1].z};
    float3 m[2];
    float3 r;
    float d = -1.0f;
    
    // wrong!

    if(bbox2[0].x >= bmax.x)
    {
        m[0].x = bmax.x; m[0].y = bbox2[0].y; m[0].z = bbox2[0].z;
        m[1].x = 1.0f;   m[1].y = 0.0f;       m[1].z = 0.0f;
    }
    else if(bbox2[0].x <= bmin.x)
    {
        m[0].x = bmin.x; m[0].y = bbox2[0].y; m[0].z = bbox2[0].z;
        m[1].x = 1.0f;   m[1].y = 0.0f;       m[1].z = 0.0f;
    }
    if(!_bsphere_test_plane(m, bbox2, &r))
    {
        return 0;
    }
    d = _ABS(r.x);
    reaction->x = r.x;
    reaction->y = r.y;
    reaction->z = r.z;

    if(bbox2[0].y >= bmax.y)
    {
        m[0].x = bbox2[0].x; m[0].y = bmax.y; m[0].z = bbox2[0].z;
        m[1].x = 0.0f;       m[1].y = 1.0f;   m[1].z = 0.0f;
    }
    else if(bbox2[0].y <= bmin.y)
    {
        m[0].x = bbox2[0].x; m[0].y = bmin.y; m[0].z = bbox2[0].z;
        m[1].x = 0.0f;       m[1].y = 1.0f;   m[1].z = 0.0f;
    }
    if(!_bsphere_test_plane(m, bbox2, &r))
    {
        return 0;
    }
    if(d > _ABS(r.y))
    {
        d = _ABS(r.y);
        reaction->x = r.x;
        reaction->y = r.y;
        reaction->z = r.z;
    }

    if(bbox2[0].z >= bmax.z)
    {
        m[0].x = bbox2[0].x; m[0].y = bbox2[0].y; m[0].z = bmax.z;
        m[1].x = 0.0f;       m[1].y = 0.0f;       m[1].z = 1.0f;
    }
    else if(bbox2[0].z <= bmin.z)
    {
        m[0].x = bbox2[0].x; m[0].y = bbox2[0].y; m[0].z = bmin.z;
        m[1].x = 0.0f;       m[1].y = 0.0f;       m[1].z = 1.0f;
    }
    if(!_bsphere_test_plane(m, bbox2, &r))
    {
        return 0;
    }
    if(d > _ABS(r.z))
    {
        d = _ABS(r.z);
        reaction->x = r.x;
        reaction->y = r.y;
        reaction->z = r.z;
    }

    return 1;
}

int
bbox_bsphere_overlap(
    float3*     bbox,
    float3*     bsphere
    )
{
    float3 bmin = {bbox[0].x - bbox[1].x, bbox[0].y - bbox[1].y, bbox[0].z - bbox[1].z};
    float3 bmax = {bbox[0].x + bbox[1].x, bbox[0].y + bbox[1].y, bbox[0].z + bbox[1].z};
    float d = 0;

    if(bsphere[0].x < bmin.x)
    {
        d += _SQR(bsphere[0].x - bmin.x);
    }
    else if(bsphere[0].x > bmax.x)
    {
        d += _SQR(bsphere[0].x - bmax.x);
    }
    if(bsphere[0].y < bmin.y)
    {
        d += _SQR((bsphere[0].y - bmin.y) * bsphere[1].x / bsphere[1].y);
    }
    else if(bsphere[0].y > bmax.y)
    {
        d += _SQR((bsphere[0].y - bmax.y) * bsphere[1].x / bsphere[1].y);
    }
    if(bsphere[0].z < bmin.z)
    {
        d += _SQR((bsphere[0].z - bmin.z) * bsphere[1].x / bsphere[1].z);
    }
    else if(bsphere[0].z > bmax.z)
    {
        d += _SQR((bsphere[0].z - bmax.z) * bsphere[1].x / bsphere[1].z);
    }
    return (d <= _SQR(bsphere[1].x)) ? 1 : 0;
}

int
bbox_bsphere_collide(
    float3*     bbox,
    float3*     bsphere,
    float3*     reaction
    )
{
    float3 bmin = {bbox[0].x - bbox[1].x, bbox[0].y - bbox[1].y, bbox[0].z - bbox[1].z};
    float3 bmax = {bbox[0].x + bbox[1].x, bbox[0].y + bbox[1].y, bbox[0].z + bbox[1].z};
    float3 m[2];

    switch(_bbox_get_voronoi_space(&bsphere[0], &bmin, &bmax))
    {
    case 0:
        if(bsphere[0].x >= bbox[0].x)
        {
            reaction->x = bmax.x - bsphere[0].x;
        }
        else
        {
            reaction->x = bmin.x - bsphere[0].x;
        }
        if(bsphere[0].y >= bbox[0].y)
        {
            reaction->y = (bmax.y - bsphere[0].y) * bsphere[1].y / bsphere[1].x;
        }
        else
        {
            reaction->y = (bmin.y - bsphere[0].y) * bsphere[1].y / bsphere[1].x;
        }
        if(bsphere[0].z >= bbox[0].z)
        {
            reaction->z = (bmax.z - bsphere[0].z) * bsphere[1].z / bsphere[1].x;
        }
        else
        {
            reaction->z = (bmin.z - bsphere[0].z) * bsphere[1].z / bsphere[1].x;
        }
        return 1;
    case 1:
        m[0].x = bmax.x; m[0].y = bsphere[0].y; m[0].z = bsphere[0].z;
        m[1].x = 1.0f;   m[1].y = 0.0f;         m[1].z = 0.0f;
        return _bsphere_test_plane(m, bsphere, reaction);
    case 2:
        m[0].x = bmin.x; m[0].y = bsphere[0].y; m[0].z = bsphere[0].z;
        m[1].x = 1.0f;   m[1].y = 0.0f;         m[1].z = 0.0f;
        return _bsphere_test_plane(m, bsphere, reaction);
    case 4:
        m[0].x = bsphere[0].x; m[0].y = bmax.y; m[0].z = bsphere[0].z;
        m[1].x = 0.0f;         m[1].y = 1.0f;   m[1].z = 0.0f;
        return _bsphere_test_plane(m, bsphere, reaction);
    case 5:
        m[0].x = bmax.x; m[0].y = bmax.y; m[0].z = bmin.z;
        m[1].x = bmax.x; m[1].y = bmax.y; m[1].z = bmax.z;
        return _bsphere_test_edge(m, bsphere, reaction);
    case 6:
        m[0].x = bmin.x; m[0].y = bmax.y; m[0].z = bmin.z;
        m[1].x = bmin.x; m[1].y = bmax.y; m[1].z = bmax.z;
        return _bsphere_test_edge(m, bsphere, reaction);
    case 8:
        m[0].x = bsphere[0].x; m[0].y = bmin.y; m[0].z = bsphere[0].z;
        m[1].x = 0.0f;         m[1].y = 1.0f;   m[1].z = 0.0f;
        return _bsphere_test_plane(m, bsphere, reaction);
    case 9:
        m[0].x = bmax.x; m[0].y = bmin.y; m[0].z = bmin.z;
        m[1].x = bmax.x; m[1].y = bmin.y; m[1].z = bmax.z;
        return _bsphere_test_edge(m, bsphere, reaction);
    case 10:
        m[0].x = bmin.x; m[0].y = bmin.y; m[0].z = bmin.z;
        m[1].x = bmin.x; m[1].y = bmin.y; m[1].z = bmax.z;
        return _bsphere_test_edge(m, bsphere, reaction);
    case 16:
        m[0].x = bsphere[0].x; m[0].y = bsphere[0].y; m[0].z = bmax.z;
        m[1].x = 0.0f;         m[1].y = 0.0f;         m[1].z = 1.0f;
        return _bsphere_test_plane(m, bsphere, reaction);
    case 17:
        m[0].x = bmax.x; m[0].y = bmin.y; m[0].z = bmax.z;
        m[1].x = bmax.x; m[1].y = bmax.y; m[1].z = bmax.z;
        return _bsphere_test_edge(m, bsphere, reaction);
    case 18:
        m[0].x = bmin.x; m[0].y = bmin.y; m[0].z = bmax.z;
        m[1].x = bmin.x; m[1].y = bmax.y; m[1].z = bmax.z;
        return _bsphere_test_edge(m, bsphere, reaction);
    case 20:
        m[0].x = bmin.x; m[0].y = bmax.y; m[0].z = bmax.z;
        m[1].x = bmax.x; m[1].y = bmax.y; m[1].z = bmax.z;
        return _bsphere_test_edge(m, bsphere, reaction);
    case 21:
        m[0].x = bmax.x; m[0].y = bmax.y; m[0].z = bmax.z;
        m[1].x = 0.0f; m[1].y = 0.0f; m[1].z = 0.0f;
        return _bsphere_test_point(m, bsphere, reaction);
    case 22:
        m[0].x = bmin.x; m[0].y = bmax.y; m[0].z = bmax.z;
        m[1].x = 0.0f; m[1].y = 0.0f; m[1].z = 0.0f;
        return _bsphere_test_point(m, bsphere, reaction);
    case 24:
        m[0].x = bmin.x; m[0].y = bmin.y; m[0].z = bmax.z;
        m[1].x = bmax.x; m[1].y = bmin.y; m[1].z = bmax.z;
       return _bsphere_test_edge(m, bsphere, reaction);
    case 25:
        m[0].x = bmax.x; m[0].y = bmin.y; m[0].z = bmax.z;
        m[1].x = 0.0f; m[1].y = 0.0f; m[1].z = 0.0f;
        return _bsphere_test_point(m, bsphere, reaction);
    case 26:
        m[0].x = bmin.x; m[0].y = bmin.y; m[0].z = bmax.z;
        m[1].x = 0.0f; m[1].y = 0.0f; m[1].z = 0.0f;
        return _bsphere_test_point(m, bsphere, reaction);
    case 32:
        m[0].x = bsphere[0].x; m[0].y = bsphere[0].y; m[0].z = bmin.z;
        m[1].x = 0.0f;         m[1].y = 0.0f;         m[1].z = 1.0f;
        return _bsphere_test_plane(m, bsphere, reaction);
    case 33:
        m[0].x = bmax.x; m[0].y = bmin.y; m[0].z = bmin.z;
        m[1].x = bmax.x; m[1].y = bmax.y; m[1].z = bmin.z;
        return _bsphere_test_edge(m, bsphere, reaction);
    case 34:
        m[0].x = bmax.x; m[0].y = bmin.y; m[0].z = bmin.z;
        m[1].x = bmax.x; m[1].y = bmax.y; m[1].z = bmin.z;
        return _bsphere_test_edge(m, bsphere, reaction);
    case 36:
        m[0].x = bmin.x; m[0].y = bmax.y; m[0].z = bmin.z;
        m[1].x = bmax.x; m[1].y = bmax.y; m[1].z = bmin.z;
        return _bsphere_test_edge(m, bsphere, reaction);
    case 37:
        m[0].x = bmax.x; m[0].y = bmax.y; m[0].z = bmin.z;
        m[1].x = 0.0f; m[1].y = 0.0f; m[1].z = 0.0f;
        return _bsphere_test_point(m, bsphere, reaction);
    case 38:
        m[1].x = bmin.x; m[1].y = bmax.y; m[1].z = bmin.z;
        m[0].x = 0.0f; m[0].y = 0.0f; m[0].z = 0.0f;
        return _bsphere_test_point(m, bsphere, reaction);
    case 40:
        m[0].x = bmin.x; m[0].y = bmin.y; m[0].z = bmin.z;
        m[1].x = bmax.x; m[1].y = bmin.y; m[1].z = bmin.z;
        return _bsphere_test_edge(m, bsphere, reaction);
    case 41:
        m[1].x = bmax.x; m[1].y = bmin.y; m[1].z = bmin.z;
        m[0].x = 0.0f; m[0].y = 0.0f; m[0].z = 0.0f;
        return _bsphere_test_point(m, bsphere, reaction);
    case 42:
        m[1].x = bmin.x; m[1].y = bmin.y; m[1].z = bmin.z;
        m[0].x = 0.0f; m[0].y = 0.0f; m[0].z = 0.0f;
        return _bsphere_test_point(m, bsphere, reaction);
    default:
        g_assert(FALSE);
    }
}

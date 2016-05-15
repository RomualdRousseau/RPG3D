/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      mesh.c
 *
 *      Copyright 2008 Romuald Rousseau <romualdrousseau@gmail.com>
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
#include <string.h>

#define CACHE_SIZE 32
#define CACHE_MISS -1

/* --- types --- */
typedef struct __VcoInfo _VcoInfo;

typedef struct __LoadNewParams _LoadNewParams;

/* --- structures --- */
struct __VcoInfo
{
    float               current_score;
    guint               active_triangles_count;
};

struct __LoadNewParams
{
    RMesh*              mesh;
    GArray*             vertice;
    GArray*             triangles;
};

/* --- variables --- */
static GLint _cache_emulator_data[CACHE_SIZE + 1];
static float _vertex_scores_cache[CACHE_SIZE];

/* --- functions --- */
/*
 * _cache_emulator_init:
 *
 */
static void
_cache_emulator_init()
{
    gint i;

    for(i = 0; i <= CACHE_SIZE; i++)
    {
        _cache_emulator_data[i] = CACHE_MISS;
    }
}

/*
 * _cache_emulator_tag:
 *
 */
static gint
_cache_emulator_tag(
    GLint       v
    )
{
    gint i;

    _cache_emulator_data[CACHE_SIZE] = v;
    for(i = 0; _cache_emulator_data[i] != v; i++);
    return (i < CACHE_SIZE) ? i : CACHE_MISS;
}

/*
 * _cache_emulator_push:
 *
 */
static void
_cache_emulator_push(
    GLint       v
    )
{
    gint j;
    guint i;

    j = _cache_emulator_tag(v);
    if(j == CACHE_MISS)
    {
        j = CACHE_SIZE - 1;
    }
    for(i = j; i > 0; i--)
    {
        _cache_emulator_data[i] = _cache_emulator_data[i - 1];
    }
    _cache_emulator_data[0] = v;
}

/*
 * _mesh_vertex_equals:
 *
 */
static gboolean
_mesh_vertex_equals(
    RVertex*    a,
    RVertex*    b
    )
{
    return (memcmp(a, b, sizeof(RVertex)) == 0) ? TRUE : FALSE;
}

/*
 * _mesh_vertex_add:
 *
 */
static void
_mesh_vertex_add(
    RVertex*    vertex,
    GArray*     vertice,
    GArray*     triangles
    )
{
    guint i;

    g_array_append_val(vertice, *vertex);
    for(i = 0; !_mesh_vertex_equals(&g_array_index(vertice, RVertex, i), vertex); i++);
    if(i < (vertice->len - 1))
    {
        g_array_remove_index(vertice, vertice->len - 1);
    }
    g_array_append_val(triangles, i);
}

/*
 * _mesh_vco_init:
 *
 */
static void
_mesh_vco_init(
    GArray*     vco_infos,
    GArray*     vertice,
    GArray*     triangles
    )
{
    _VcoInfo vi;
    _VcoInfo* pvi;
    guint i, j;

    for(i = 0; i < CACHE_SIZE; i++)
    {
        if(i < 3)
        {
            _vertex_scores_cache[i] = 0.75f;
        }
        else
        {
            _vertex_scores_cache[i] = powf(1.0f - (i - 3) * (1.0f / (CACHE_SIZE - 3)), 1.5f);
        }
    }
    for(i = 0; i < vertice->len; i++)
    {
        vi.active_triangles_count = 0;
        for(j = 0; j < triangles->len; j++)
        {
            if(i == g_array_index(triangles, GLuint, j))
            {
                vi.active_triangles_count++;
            }
        }
        g_array_append_val(vco_infos, vi);
    }
    for(i = 0; i < vco_infos->len; i++)
    {
        pvi = &g_array_index(vco_infos, _VcoInfo, i);
        pvi->current_score = 2.0f * powf(pvi->active_triangles_count, -0.5f);
    }
}

/*
 * _mesh_vco_refresh_all_vertex_scores:
 *
 */
static void
_mesh_vco_refresh_all_vertex_scores(
    GArray*     vco_infos
    )
{
    GLuint e;
    _VcoInfo* vi;
    guint i;

    for(i = 0; i < CACHE_SIZE; i++)
    {
        e = _cache_emulator_data[i];
        if(e != CACHE_MISS)
        {
            vi = &g_array_index(vco_infos, _VcoInfo, e);
            if(vi->active_triangles_count <= 0)
            {
                vi->current_score = -1.0f;
            }
            else
            {
                vi->current_score = 2.0f * powf(vi->active_triangles_count, -0.5f) + _vertex_scores_cache[i];
            }
        }
    }
}

/*
 * _mesh_vco_get_triangle_score:
 *
 */
static float
_mesh_vco_get_triangle_score(
    GArray*     vco_infos,
    GArray*     triangles,
    guint       i
    )
{
    float score = 0.0f;
    GLuint e;
    _VcoInfo* vi;
    guint j;

    for(j = i; j < i + 3; j++)
    {
        e = g_array_index(triangles, GLuint, j);
        vi = &g_array_index(vco_infos, _VcoInfo, e);
        score += vi->current_score;
    }
    return score;
}

/*
 * _mesh_vco_find_best_score:
 *
 */
static guint
_mesh_vco_find_best_score(
    GArray*     vco_infos,
    GArray*     triangles
    )
{
    guint i;
    float s;
    float best_s;
    guint best_i;

    best_i = 0;
    best_s = _mesh_vco_get_triangle_score(vco_infos, triangles, 0);
    for(i = 3; i < triangles->len; i += 3)
    {
        s = _mesh_vco_get_triangle_score(vco_infos, triangles, i);
        if(best_s < s)
        {
            best_i = i;
            best_s = s;
        }
    }
    return best_i;
}

/*
 * _mesh_vco_optimize:
 *
 */
static void
_mesh_vco_optimize(
    GArray*     vertice,
    GArray*     triangles,
    GArray*     returned_vertice,
    GArray*     returned_triangles
    )
{
    GArray* vco_infos;
    GArray* unvisited_triangles;
    GLuint e;
    RVertex* v;
    _VcoInfo* vi;
    guint i;
    GLuint best;

    _cache_emulator_init();

    vco_infos = g_array_sized_new(FALSE, TRUE, sizeof(_VcoInfo), vertice->len);
    _mesh_vco_init(vco_infos, vertice, triangles);

    unvisited_triangles = g_array_sized_new(FALSE, TRUE, sizeof(GLuint), triangles->len);
    g_array_append_vals(unvisited_triangles, triangles->data, triangles->len);

    while(unvisited_triangles->len > 0)
    {
        best = _mesh_vco_find_best_score(vco_infos, unvisited_triangles);
        for(i = best; i < best + 3; i++)
        {
            e = g_array_index(unvisited_triangles, GLuint, i);

            v = &g_array_index(vertice, RVertex, e);
            _mesh_vertex_add(v, returned_vertice, returned_triangles);

            vi = &g_array_index(vco_infos, _VcoInfo, e);
            vi->active_triangles_count--;

            _cache_emulator_push(e);
        }
        g_array_remove_range(unvisited_triangles, best, 3);
        _mesh_vco_refresh_all_vertex_scores(vco_infos);
    }

    g_array_free(unvisited_triangles, TRUE);
    g_array_free(vco_infos, TRUE);

    g_array_unref(vertice);
    g_array_unref(triangles);
}

/*
 * _mesh_compute_acmr:
 *
 */
#ifdef DEBUG
static float
_mesh_compute_acmr(
    GArray*     triangles
    )
{
    float misses = 0.0f;
    GLuint e;
    guint i;

    _cache_emulator_init();

    for(i = 0; i < triangles->len; i++)
    {
        e = g_array_index(triangles, GLuint, i);
        if(_cache_emulator_tag(e) == CACHE_MISS)
        {
            misses += 1.0f;
            _cache_emulator_push(e);
        }
    }
    return misses * 3.0f / (float) triangles->len;
}
#endif

/*
 * _mesh_new_delegate:
 *
 */
static gpointer
_mesh_new_delegate(
    _LoadNewParams*      params
    )
{
    params->mesh->vertice_count = params->vertice->len;
    glGenBuffersARB(1, &params->mesh->vertice_vbo);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, params->mesh->vertice_vbo);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, params->mesh->vertice_count * sizeof(RVertex), params->vertice->data, GL_STATIC_DRAW_ARB);

    params->mesh->triangles_count = params->triangles->len;
    glGenBuffersARB(1, &params->mesh->triangles_vbo);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, params->mesh->triangles_vbo);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, params->mesh->triangles_count * sizeof(GLuint), params->triangles->data, GL_STATIC_DRAW_ARB);

    g_free(params);
    return NULL;
}

/*
 * _mesh_free_delegate:
 *
 */
static gpointer
_mesh_free_delegate(
    RMesh*          mesh
    )
{
    glDeleteBuffersARB(1, &mesh->vertice_vbo);
    glDeleteBuffersARB(1, &mesh->triangles_vbo);

    g_free(mesh);
    return NULL;
}

/**
 * r_mesh_new:
 *
 *
 */
RMesh*
r_mesh_new(
    RMaterial*  skin,
    RVertex*    vertice,
    guint       vertice_count
    )
{
    _LoadNewParams* params;
    RMesh* mesh;
    GArray* v;
    GArray* e;
    GArray* ov;
    GArray* oe;
    guint i;

    g_assert(GLEW_ARB_vertex_buffer_object);
    g_assert(skin != NULL);
    g_assert(vertice != NULL);

#ifdef DEBUG
    g_debug("Vertice: %d", vertice_count);
#endif
    v = g_array_new(FALSE, TRUE, sizeof(RVertex));
    e = g_array_new(FALSE, TRUE, sizeof(GLuint));
    for(i = 0; i < vertice_count; i++)
    {
        _mesh_vertex_add(&vertice[i], v, e);
    }
#ifdef DEBUG
    g_debug("Vertice/triangles: %d/%d", v->len, e->len);
#endif

    if(vertice_count >= CACHE_SIZE)
    {
#ifdef DEBUG
        g_debug("ACMR before optimization: %f", _mesh_compute_acmr(e));
#endif
        ov = g_array_sized_new(FALSE, TRUE, sizeof(RVertex), v->len);
        oe = g_array_sized_new(FALSE, TRUE, sizeof(GLuint), e->len);
        _mesh_vco_optimize(v, e, ov, oe);
#ifdef DEBUG
        g_debug("ACMR after optimization: %f", _mesh_compute_acmr(oe));
#endif
    }
    else
    {
#ifdef DEBUG
        g_debug("optimization disable for this mesh");
#endif
        ov = v;
        oe = e;
    }

    mesh = g_new0(RMesh, 1);
    memcpy(&mesh->skin, skin, sizeof(RMaterial));

    params = g_new0(_LoadNewParams, 1);
    params->mesh = mesh;
    params->vertice = ov;
    params->triangles = oe;
    r_renderer_execute((GThreadFunc) _mesh_new_delegate, params);

    g_array_free(ov, TRUE);
    g_array_free(oe, TRUE);
    return mesh;
}

/**
 * r_mesh_free:
 *
 *
 */
void
r_mesh_free(
    RMesh*       mesh
    )
{
    g_assert(mesh != NULL);

    r_renderer_execute((GThreadFunc) _mesh_free_delegate, mesh);
}

/**
 * r_mesh_draw:
 *
 *
 */
void
r_mesh_draw(
    RMesh*       mesh
    )
{
    g_assert(mesh != NULL);

    if(mesh->material.texture != 0)
    {
        glBindTexture(GL_TEXTURE_2D, mesh->material.texture);
        glEnable(GL_TEXTURE_2D);
    }
    else
    {
        glMaterialfv(GL_FRONT, GL_DIFFUSE, (gfloat*)&mesh->material.color);
    }

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->vertice_vbo);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->triangles_vbo);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(RVertex), (gconstpointer)&((RVertex*)0)->vertex);

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(RVertex), (gconstpointer)&((RVertex*)0)->normal);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(RVertex), (gconstpointer)&((RVertex*)0)->texcoord);

    glDrawElements(GL_TRIANGLES, mesh->triangles_count, GL_UNSIGNED_INT, (gconstpointer)0);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glDisable(GL_TEXTURE_2D);
}



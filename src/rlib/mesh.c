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
#include <memory.h>

#define SELF(m) ((_RMesh*) (m))
#define VBO_OFFSET0(s) (gconstpointer) ((guint*)NULL + (s))
#define VBO_OFFSET(s, p) (gconstpointer)&((s*)0)->p

/* --- types --- */
typedef struct __RMesh _RMesh;

/* --- structures --- */
struct __RMesh
{
/* public */
    guint                   vertice_count;
    guint                   frames_count;
    guint                   parts_count;
    guint                   triangles_count;
    RMeshElement**          frames;
    RMeshPart*              parts;
    guint*                  triangles;
    gfloat                  anim_time;
/* private */
    GLuint                  vertice_vbo;
    GLuint                  triangles_vbo;
};

/* --- functions --- */
/*
 * _r_mesh_frame_lerp:
 *
 */
static inline gboolean
_mesh_frame_lerp(
    _RMesh*         self,
    guint           frame_first,
    guint           frame_last,
    guint           frame_fps,
    gboolean        repeat_mode,
    RMeshElement*   result
    )
{
    gboolean animating = TRUE;
    guint frame_current;
    guint frame_range;
    RMeshElement* v1;
    RMeshElement* v2;
    RMeshElement* v3;
    guint i;
    gfloat t;
    
    self->anim_time += 0.000001f * frame_fps * game->frame_time;
    
    frame_range = frame_last - frame_first;
    frame_current = (guint) self->anim_time;
    
    if(repeat_mode)
    {
        if(frame_current > frame_range)
        {
            self->anim_time = 0.0f;
            frame_current = 0;
        }
    }
    else
    {
        if(frame_current >= frame_range)
        {
            self->anim_time = frame_range;
            frame_current = frame_range;
            animating = FALSE;
        }
    }
    
    t = self->anim_time - (gfloat) frame_current;
    if(t > 1.0f)
    {
        t = 1.0f;
    }
    
    v1 = self->frames[frame_first + frame_current];
    v2 = self->frames[frame_first + (frame_current + 1) % (frame_range + 1)];
    v3 = result;
    for(i = 0; i < self->vertice_count; i++, v1++, v2++, v3++)
    {
        lerp3(&v1->point, &v2->point, t, &v3->point);
        lerp3(&v1->normal, &v2->normal, t, &v3->normal);
        v3->texcoord.x = v1->texcoord.x;
        v3->texcoord.y = v1->texcoord.y;
    }
    
    return animating;
}

/*
 * _r_mesh_destroy:
 *
 */
static void
_r_mesh_free(
    _RMesh*         self
    )
{
    g_free(self->triangles);
    g_free(self->parts);
    g_free(self->frames[0]);
    g_free(self->frames);
    g_slice_free(_RMesh, self);
}

/*
 * _r_mesh_concat:
 *
 */
static void
_r_mesh_concat(
    _RMesh*              mesh1,
    _RMesh*              mesh2
    )
{
    guint i;

    g_assert(mesh1 != NULL);
    g_assert(mesh2 != NULL);
    g_assert(mesh1->vertice_count == mesh2->vertice_count);
    
    mesh1->frames_count++;
    mesh1->frames = g_renew(RMeshElement*, mesh1->frames, mesh1->frames_count);
    mesh1->frames[0] =  g_renew(RMeshElement, mesh1->frames[0], mesh1->frames_count * mesh1->vertice_count);
    
    for(i = 1; i < mesh1->frames_count; i++)
    {
        mesh1->frames[i] = &mesh1->frames[i - 1][mesh1->vertice_count];
    }
    
    memcpy(mesh1->frames[mesh1->frames_count - 1], mesh2->frames[0], mesh1->vertice_count * sizeof(RMeshElement));
}

/*
 * _r_mesh_new_delegate:
 *
 */
static gpointer
_mesh_new_delegate(
    _RMesh*         self
    )
{
    glGenBuffersARB(1, &self->vertice_vbo);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, self->vertice_vbo);
    if(self->frames_count == 1)
    {
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, self->vertice_count * sizeof(RMeshElement), self->frames[0], GL_STATIC_DRAW_ARB);
    }
    else
    {
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, self->vertice_count * sizeof(RMeshElement), NULL, GL_STREAM_DRAW_ARB);
    }
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);   

    glGenBuffersARB(1, &self->triangles_vbo);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, self->triangles_vbo);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, self->triangles_count * 3 * sizeof(guint), self->triangles, GL_STATIC_DRAW_ARB);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

    return NULL;
}

/*
 * _r_mesh_free_delegate:
 *
 */
static gpointer
_mesh_free_delegate(
    _RMesh*         self
    )
{
    glDeleteBuffersARB(1, &self->vertice_vbo);
    glDeleteBuffersARB(1, &self->triangles_vbo);
    _r_mesh_free(self);
    return NULL;
}

/**
 * r_mesh_element_equals:
 *
 **/
gboolean
r_mesh_element_equals(
    RMeshElement* e1,
    RMeshElement* e2
    )
{
    if(memcmp(e1, e2, sizeof(RMeshElement)) == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
 * r_mesh_element_insert:
 *
 **/
gboolean
r_mesh_element_insert(
    RMeshElement*       elements,
    guint               index,
    RMeshElement*       element,
    guint*              result_index
    )
{
    gushort i;
    
    memcpy(&elements[index], element, sizeof(RMeshElement));
    for(i = 0; !r_mesh_element_equals(&elements[i], element); i++);
    *result_index = i;
    return (index == i);
}

/**
 * r_mesh_element_replace:
 *
 **/
void
r_mesh_element_replace(
    RMeshElement*       elements,
    guint               index,
    RMeshElement*       element
    )
{
    memcpy(&elements[index], element, sizeof(RMeshElement));
}

/**
 * r_mesh_new:
 *
 **/
RMesh*
r_mesh_new(
    guint                   frames_count,
    guint                   vertice_count,
    guint                   parts_count,
    guint                   triangles_count
    )
{
    _RMesh* self;
    guint i;
    
    g_assert(frames_count > 0);
    g_assert(vertice_count > 0);
    g_assert(triangles_count > 0);
    
    self = g_slice_new0(_RMesh);
    self->vertice_count = vertice_count;
    self->frames_count = frames_count;
    self->frames = g_new0(RMeshElement*, frames_count);
    self->frames[0] =  g_new0(RMeshElement, frames_count * vertice_count);
    self->parts_count = parts_count;
    self->parts = g_new0(RMeshPart, parts_count);
    self->triangles_count = triangles_count;
    self->triangles = g_new0(guint, triangles_count * 3);
    
    self->parts[0].skin = NULL;
    self->parts[0].offset = 0;
    self->parts[0].count = self->triangles_count * 3;
    
    for(i = 1; i < self->frames_count; i++)
    {
        self->frames[i] = &self->frames[i - 1][vertice_count];
    }
    
    return (RMesh*) self;
}

/**
 * r_mesh_new_from_file:
 *
 **/
RMesh*
r_mesh_new_from_file(
    const gchar*            file_name,
    RMaterial*              default_skin
    )
{
    _RMesh* self;
    guint   i;
    
    g_assert(GLEW_ARB_vertex_buffer_object);
    g_assert(file_name != NULL);

    self = SELF(r_modules_lookup(file_name)->load_from_file(file_name));
    if(self == NULL)
    {
        return NULL;
    }
    
    if(default_skin != NULL)
    {
        for(i = 0; i < self->parts_count; i++)
        {
            self->parts[i].skin = default_skin;
        }
    }
    
    for(i = 0; i < self->parts_count; i++)
    {
        g_assert(self->parts[i].skin != NULL);
    }

    r_renderer_execute((GThreadFunc) _mesh_new_delegate, self);
    
    return (RMesh*) self;
}

/**
 * r_mesh_new_from_files:
 *
 **/
RMesh*
r_mesh_new_from_files(
    const gchar**           file_names,
    RMaterial*              default_skin
    )
{
    _RMesh* self;
    _RMesh* other;
    guint i;    

    g_assert(GLEW_ARB_vertex_buffer_object);
    g_assert(file_names != NULL);
    g_assert(file_names[0] != NULL);
    
    self = SELF(r_modules_lookup(file_names[0])->load_from_file(file_names[0]));
    if(self == NULL)
    {
        return NULL;
    }
    
    for(i = 1; file_names[i] != NULL; i++)
    {
        other = SELF(r_modules_lookup(file_names[i])->load_from_file(file_names[i]));
        if(other == NULL)
        {
            return NULL;
        }
        _r_mesh_concat(self, other);
        _r_mesh_free(other);
    }
    
    if(default_skin != NULL)
    {
        for(i = 0; i < self->parts_count; i++)
        {
            self->parts[i].skin = default_skin;
        }
    }
    
    for(i = 0; i < self->parts_count; i++)
    {
        g_assert(self->parts[i].skin != NULL);
    }

    r_renderer_execute((GThreadFunc) _mesh_new_delegate, self);
    
    return (RMesh*) self;
}

/**
 * r_mesh_free:
 *
 **/
void
r_mesh_free(
    RMesh*              mesh
    )
{
    if(mesh != NULL)
    {
        r_renderer_execute((GThreadFunc) _mesh_free_delegate, SELF(mesh));
    }
}

/**
 * r_mesh_compute_bbox:
 *
 **/
void
r_mesh_compute_bbox(
    RMesh*              mesh,
    guint               frame,
    float3*             bbox_result
    )
{
    RMeshElement* p;
    float3 min = {0.0f, 0.0f, 0.0f};
    float3 max = {0.0f, 0.0f, 0.0f};
    
    g_assert(mesh != NULL);
    g_assert(frame < mesh->frames_count);
    g_assert(bbox_result != NULL);

    p = SELF(mesh)->frames[frame];
    min.x = p->point.x; 
    min.y = p->point.y; 
    min.z = p->point.z;
    max.x = p->point.x; 
    max.y = p->point.y; 
    max.z = p->point.z;  
    p++;
    
    while(p < (const RMeshElement*) &SELF(mesh)->frames[frame][mesh->vertice_count])
    {
        min.x = MIN(min.x, p->point.x); 
        min.y = MIN(min.y, p->point.y); 
        min.z = MIN(min.z, p->point.z); 
        max.x = MAX(max.x, p->point.x); 
        max.y = MAX(max.y, p->point.y); 
        max.z = MAX(max.z, p->point.z);  
        p++;
    }
    
    bbox_result[0].x = (max.x + min.x) / 2.0f;
    bbox_result[0].y = (max.y + min.y) / 2.0f;
    bbox_result[0].z = (max.z + min.z) / 2.0f;
    bbox_result[1].x = _ABS(max.x - min.x) / 2.0f;
    bbox_result[1].y = _ABS(max.y - min.y) / 2.0f;
    bbox_result[1].z = _ABS(max.z - min.z) / 2.0f;
}

/**
 * r_mesh_collide:
 *
 **/
gboolean
r_mesh_collide(
    RMesh*                  mesh,
    guint                   frame,
    float3*                 bbox,
    float3*                 reaction
    )
{
    gboolean result = FALSE;
    guint* i;
    guint j;
    float3 r = {0.0f, 0.0f, 0.0f};
    float3 s = {1.0f / bbox[1].x, 1.0f / bbox[1].y, 1.0f / bbox[1].z};
    float3 t[3];
    float3* a;
    float3 b;
    float4 p;
    float l;
    
    g_assert(mesh != NULL);
    g_assert(frame < mesh->frames_count);
    g_assert(bbox != NULL);
    g_assert(reaction != NULL);

    for(i = &SELF(mesh)->triangles[0]; i < (const guint*) &SELF(mesh)->triangles[mesh->triangles_count * 3];)
    {
        for(j = 0; j < 3; j++, i++)
        {
            a = &mesh->frames[frame][*i].point;
            t[j].x = (a->x - bbox[0].x) * s.x;
            t[j].y = (a->y - bbox[0].y) * s.y;
            t[j].z = (a->z - bbox[0].z) * s.z;
        }
        
        r_triangle_to_plane(t, &p);
        if((p.w < 0) || (p.w > 1.0f))
        {
            continue;
        }

        b.x = -p.x * p.w;
        b.y = -p.y * p.w;
        b.z = -p.z * p.w;
        if(!r_triangle_contain_point(t, &b))
        {
            continue;
        }
        
        l = 1.0f - p.w;
        r.x += l * p.x;
        r.y += l * p.y;
        r.z += l * p.z;
        result = TRUE;
    }
    
    if(result)
    {
        reaction->x += r.x * bbox[1].x;
        reaction->y += r.y * bbox[1].y;
        reaction->z += r.z * bbox[1].z;
    }
    
    return result;
}

/**
 * r_mesh_draw:
 *
 **/
void
r_mesh_draw(
    float4x4*               view,
    RMesh*                  mesh
    )
{
    RMeshPart* part;
    
    g_assert(mesh != NULL);
    
    if(view != NULL)
    {
        glLoadMatrixf((GLfloat*) view);
    }

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, SELF(mesh)->vertice_vbo);
    if(mesh->frames_count > 1)
    {
        RMeshElement* buffer = (RMeshElement*) glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB);
        memcpy(buffer, SELF(mesh)->frames[0], SELF(mesh)->vertice_count * sizeof(RMeshElement));
        glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
    }
    
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, SELF(mesh)->triangles_vbo);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(RMeshElement), VBO_OFFSET(RMeshElement, point));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(RMeshElement), VBO_OFFSET(RMeshElement, normal));

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(RMeshElement), VBO_OFFSET(RMeshElement, texcoord));

    for(part = &SELF(mesh)->parts[0]; part < (const RMeshPart*) &SELF(mesh)->parts[SELF(mesh)->parts_count]; part++)
    {
        if(part->skin->texture != R_TEXTURE_NONE)
        {
            glBindTexture(GL_TEXTURE_2D, part->skin->texture);
            glEnable(GL_TEXTURE_2D);
        }
        else
        {
            glMaterialfv(GL_FRONT, GL_DIFFUSE, (gfloat*) &part->skin->color);
            glDisable(GL_TEXTURE_2D);
        }
        glDrawElements(GL_TRIANGLES, part->count, GL_UNSIGNED_INT, VBO_OFFSET0(part->offset));
    }
    
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glDisable(GL_TEXTURE_2D);
}

/**
 * r_mesh_draw_frame:
 *
 **/
gboolean
r_mesh_draw_full(
    float4x4*               view,
    RMesh*                  mesh,
    guint                   frame_first,
    guint                   frame_last,
    guint                   frame_fps,
    gboolean                repeat_mode
    )
{
    gboolean animating = TRUE;
    RMeshPart* part;
    
    g_assert(mesh != NULL);
    g_assert(frame_first <= frame_last);
    g_assert(frame_last < mesh->frames_count);
    g_assert(frame_fps > 0);
    
    if(view != NULL)
    {
        glLoadMatrixf((GLfloat*) view);
    }
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, SELF(mesh)->vertice_vbo);
    if(SELF(mesh)->frames_count > 1)
    {
        RMeshElement* buffer = (RMeshElement*) glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB);
        animating = _mesh_frame_lerp(SELF(mesh), frame_first, frame_last, frame_fps, repeat_mode, buffer);
        glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
    }
    
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, SELF(mesh)->triangles_vbo);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(RMeshElement), VBO_OFFSET(RMeshElement, point));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(RMeshElement), VBO_OFFSET(RMeshElement, normal));

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(RMeshElement), VBO_OFFSET(RMeshElement, texcoord));
    
    for(part = &SELF(mesh)->parts[0]; part < (const RMeshPart*) &SELF(mesh)->parts[SELF(mesh)->parts_count]; part++)
    {
        if(part->skin->texture != R_TEXTURE_NONE)
        {
            glBindTexture(GL_TEXTURE_2D, part->skin->texture);
            glEnable(GL_TEXTURE_2D);
        }
        else
        {
            glMaterialfv(GL_FRONT, GL_DIFFUSE, (gfloat*) &part->skin->color);
            glDisable(GL_TEXTURE_2D);
        }
        glDrawElements(GL_TRIANGLES, part->count, GL_UNSIGNED_INT, VBO_OFFSET0(part->offset));
    }

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glDisable(GL_TEXTURE_2D);
    
    return animating;
}

/**
 * r_meshgroup_new:
 *
 **/
RMeshGroup*
r_meshgroup_new()
{
    RMeshGroup* self;
    
    self = g_slice_new0(RMeshGroup);
    self->groups = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)r_mesh_free);
    return self;
}

/**
 * r_meshgroup_new_from_file:
 *
 **/
RMeshGroup*
r_meshgroup_new_from_file(
    const gchar*            file_name,
    RMaterial*              default_skin
    )
{
    RMeshGroup* self;
    RMesh* group;
    GHashTableIter iter;
    guint   i;
    
    g_assert(GLEW_ARB_vertex_buffer_object);
    g_assert(file_name != NULL);

    self = (RMeshGroup*)r_modules_lookup(file_name)->load_from_file(file_name);
    if(self == NULL)
    {
        return NULL;
    }
    
    g_hash_table_iter_init(&iter, self->groups);
    while (g_hash_table_iter_next(&iter, NULL, (gpointer)&group))
    {
        if(default_skin != NULL)
        {
            for(i = 0; i < group->parts_count; i++)
            {
                group->parts[i].skin = default_skin;
            }
        }
        
        r_renderer_execute((GThreadFunc) _mesh_new_delegate, group);
    }

    return self;
}

/**
 * r_meshgroup_free:
 *
 **/
void
r_meshgroup_free(
    RMeshGroup*             meshgroup
    )
{
    g_hash_table_destroy(meshgroup->groups);
}

/**
 * r_meshgroup_get:
 *
 **/
RMesh*
r_meshgroup_get(
    RMeshGroup*             meshgroup,
    const gchar*            group_name
    )
{
    return g_hash_table_lookup(meshgroup->groups, group_name);
}

/*
 *      md2.c
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
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <memory.h>
#include "md2_normals.h"

#define MD2_IDENT                (('2'<<24) + ('P'<<16) + ('D'<<8) + 'I')
#define MD2_VERSION              8

struct _MD2Header
{
    gint32          ident;
    gint32          version;

    gint32          skinwidth;
    gint32          skinheight;
    gint32          framesize;

    gint32          num_skins;
    gint32          num_xyz;
    gint32          num_st;
    gint32          num_tris;
    gint32          num_glcmds;
    gint32          num_frames;

    gint32          ofs_skins;
    gint32          ofs_st;
    gint32          ofs_tris;
    gint32          ofs_frames;
    gint32          ofs_glcmds;
    gint32          ofs_end;
};
typedef struct _MD2Header MD2Header;

struct _MD2Vertex
{
    guint8          point[3];
    guint8          normal;

};
typedef struct _MD2Vertex MD2Vertex;

struct _MD2Texcoord
{
    gint16          s;
    gint16          t;
};
typedef struct _MD2Texcoord MD2Texcoord;

struct _MD2Triangle
{
    gint16          index_vertex[3];
    gint16          index_texcoord[3];

};
typedef struct _MD2Triangle MD2Triangle;

struct _MD2Frame
{
    gfloat          scale[3];
    gfloat          translate[3];
    gchar           name[16];
    MD2Vertex       vertices[1];

};
typedef struct _MD2Frame MD2Frame;

static gpointer
_load_from_file(
    const gchar*      file_name
    )
{
    RMesh* mesh;
    FILE* stream;
    guint i, j, k;
    MD2Header header;
    MD2Texcoord* texcoords;
    MD2Triangle* triangles;
    MD2Frame* frame;
    RMeshElement element;

    stream = fopen(file_name, "rb");
    if(stream == NULL)
    {
        return NULL;
    }

    fread(&header, sizeof(MD2Header), 1, stream);

    if(header.ident != MD2_IDENT || header.version != MD2_VERSION)
    {
        fclose(stream);
        return NULL;
    }

    texcoords = g_new0(MD2Texcoord, header.num_st);
    fseek(stream, header.ofs_st, SEEK_SET);
    fread(texcoords, header.num_st * sizeof(MD2Texcoord), 1, stream);

    triangles = g_new0(MD2Triangle, header.num_tris);
    fseek(stream, header.ofs_tris, SEEK_SET);
    fread(triangles, header.num_tris * sizeof(MD2Triangle), 1, stream);

    frame = g_new0(MD2Frame, header.framesize);
    fseek(stream, header.ofs_frames, SEEK_SET);

    mesh = r_mesh_new(header.num_frames, header.num_tris * 3, 1, header.num_tris);
    mesh->vertice_count = 0;
    
    for(i = 0; i < header.num_frames; i++)
    {
        fread(frame, header.framesize, 1, stream);

        for(j = 0; j < header.num_tris * 3; j++)
        {
            k = triangles[j / 3].index_vertex[j % 3];
            element.point.x = (-(gfloat) frame->vertices[k].point[0] * frame->scale[0] - frame->translate[0]) * 0.02f;
            element.point.y = (+(gfloat) frame->vertices[k].point[2] * frame->scale[2] + frame->translate[2]) * 0.02f;
            element.point.z = (-(gfloat) frame->vertices[k].point[1] * frame->scale[1] - frame->translate[1]) * 0.02f;

            element.normal.x = -fast_normals[frame->vertices[k].normal][0];
            element.normal.y = +fast_normals[frame->vertices[k].normal][2];
            element.normal.z = -fast_normals[frame->vertices[k].normal][1];

            k = triangles[j / 3].index_texcoord[j % 3];
            element.texcoord.x = (gfloat) texcoords[k].s / header.skinwidth;
            element.texcoord.y = (gfloat) texcoords[k].t / header.skinheight;

            if(i == 0)
            {
                if(r_mesh_element_insert(mesh->frames[0], mesh->vertice_count, &element, &mesh->triangles[j]))
                {
                    mesh->vertice_count++;
                }
            }
            else
            {
                r_mesh_element_replace(mesh->frames[i], mesh->triangles[j], &element);
            }
        }
    }

    g_free(frame);
    g_free(triangles);
    g_free(texcoords);
    fclose(stream);

    return mesh;
}

static RModule singleton =
{
    _load_from_file
};

static RModule*
_get_instance()
{
    return &singleton;
}

static gboolean
_accept_file(
    const gchar*      file_name
    )
{
    return g_str_has_suffix(file_name, ".md2");
}

RModuleFactory md2_module_factory =
{
    "md2",
    "md2 image loader",
    _get_instance,
    _accept_file
};

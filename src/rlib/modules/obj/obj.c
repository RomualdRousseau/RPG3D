/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      obj.c
 *
 *      Copyright 2009 Romuald Rousseau <romualdrousseau@msn.com>
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
#include <string.h>
#include <errno.h>

enum
{
    R_SYMBOL_NONE,
    R_SYMBOL_IDENT,
    R_SYMBOL_INT,
    R_SYMBOL_FLOAT,
    R_SYMBOL_STRING,
    R_SYMBOL_EOL,
    R_SYMBOL_EOF
};

struct _OBJPart
{
    RMaterial*      material;
    guint           offset;
    guint           count;
};
typedef struct _OBJPart OBJPart;

struct _OBJTriangle
{
    gint           index_point[3];
    gint           index_normal[3];
    gint           index_texcoord[3];

};
typedef struct _OBJTriangle OBJTriangle;

struct _OBJCompiler
{
    guint           line;
    guint           pos;
    gchar*          token;
    gchar**         buffer;
    
    GArray*         points;
    GArray*         texcoords;
    GArray*         normals;
    GArray*         parts;
    GArray*         triangles;
    
    GHashTable*     objects;
    gchar*          current_object_name;
    gboolean        output;
};
typedef struct _OBJCompiler OBJCompiler;

const gchar* symbol_string[] =
{
    "none",
    "identifier",
    "integer",
    "float",
    "string",
    "end of line",
    "end of file"
};

static gboolean
_is_symbol_equals(
    gchar*          token,
    guint           symbol,
    gpointer        value
    )
{
    switch(symbol)
    {
        case R_SYMBOL_NONE:
            if(token != NULL)
            {
                if(token[0] == '\0')
                {
                    return TRUE;
                }
            }
            break;
            
        case R_SYMBOL_IDENT:
            if(token != NULL && token[0] != '\0')
            {
                if(!g_strcmp0(token, (gchar*) value))
                {
                    return TRUE;
                }
            }
            break;
            
        case R_SYMBOL_INT:
            if(token != NULL && token[0] != '\0')
            {
                *((gint*) value) = g_ascii_strtoll(token, NULL, 10) - 1;
                if(errno != ERANGE)
                {
                    return TRUE;
                }
            }
            break;
            
        case R_SYMBOL_FLOAT:
            if(token != NULL && token[0] != '\0')
            {
                *((gfloat*) value) = g_ascii_strtod(token, NULL);
                if(errno != ERANGE)
                {
                    return TRUE;
                }
            }
            break;
            
        case R_SYMBOL_STRING:
            if(token != NULL && token[0] != '\0')
            {
                *((gchar**) value) = token;
                return TRUE;
            }
            break;
            
        case R_SYMBOL_EOL:
            if(token == NULL)
            {
                return TRUE;
            }
            break;
            
        case R_SYMBOL_EOF:
            if(token == NULL)
            {
                return TRUE;
            }
            break;
    }
    return FALSE;
}

static gboolean
_get_symbol(
    OBJCompiler*    compiler,
    FILE*           stream
    )
{
    gchar buffer[256];
    
    if(stream != NULL)
    {
        do
        {
            g_strfreev(compiler->buffer);
            compiler->buffer = NULL;
            
            if(!fgets(buffer, sizeof(buffer), stream))
            {
                compiler->token = NULL;
                return FALSE;
            }
            
            compiler->buffer = g_strsplit_set(g_strchomp(g_strdelimit(buffer, "\t\r\n", ' ')), "/ ", -1);
            compiler->line++;
            compiler->pos = 0;
            compiler->token = compiler->buffer[compiler->pos++];
        }
        while(compiler->token == NULL || compiler->token[0] == '#');
    }
    else
    {
        if(compiler->token != NULL)
        {
            compiler->token = compiler->buffer[compiler->pos++];
        }
    }
    return TRUE;
}

static gboolean
_accept(
    OBJCompiler*    compiler,
    guint           symbol,
    gpointer        value
    )
{
    if(_is_symbol_equals(compiler->token, symbol, value))
    {
        _get_symbol(compiler, NULL);
        return TRUE;
    }
    return FALSE;
}

static void
_expect(
    OBJCompiler*    compiler,
    guint           symbol,
    gpointer        value
    )
{
    if(!_accept(compiler, symbol, value))
    {
        g_error("line %d: %s expected at pos %d", compiler->line, symbol_string[symbol], compiler->pos);
    }
}

static RMesh*
_output_mesh(
    OBJCompiler*    compiler
    )
{
    RMesh* mesh;
    guint i;
    gint k;
    RMeshElement element;
    
    mesh = r_mesh_new(1, compiler->triangles->len * 3, compiler->parts->len, compiler->triangles->len);
    mesh->vertice_count = 0;
    
    for(i = 0; i < compiler->parts->len; i++)
    {
        mesh->parts[i].skin = g_array_index(compiler->parts, OBJPart, i).material;
        mesh->parts[i].offset = g_array_index(compiler->parts, OBJPart, i).offset;
        mesh->parts[i].count = g_array_index(compiler->parts, OBJPart, i).count;
    }
    
    for(i = 0; i < compiler->triangles->len * 3; i++)
    {
        k = g_array_index(compiler->triangles, OBJTriangle, i / 3).index_point[i % 3];
        element.point.x = g_array_index(compiler->points, float3, k).x;
        element.point.y = g_array_index(compiler->points, float3, k).y;
        element.point.z = g_array_index(compiler->points, float3, k).z;

        k = g_array_index(compiler->triangles, OBJTriangle, i / 3).index_normal[i % 3];
        element.normal.x = g_array_index(compiler->normals, float3, k).x;
        element.normal.y = g_array_index(compiler->normals, float3, k).y;
        element.normal.z = g_array_index(compiler->normals, float3, k).z;

        k = g_array_index(compiler->triangles, OBJTriangle, i / 3).index_texcoord[i % 3];
        if(k >= 0)
        {
            element.texcoord.x = g_array_index(compiler->texcoords, float2, k).x;
            element.texcoord.y =  1.0f - g_array_index(compiler->texcoords, float2, k).y;
        }
        else
        {
            element.texcoord.x = 0.0f;
            element.texcoord.y = 0.0f;
        }

        if(r_mesh_element_insert(mesh->frames[0], mesh->vertice_count, &element, &mesh->triangles[i]))
        {
            mesh->vertice_count++;
        }
    }
    
    if(compiler->current_object_name != NULL)
    {
        g_hash_table_insert(compiler->objects, compiler->current_object_name, mesh);
    }
    g_array_remove_range(compiler->parts, 0, compiler->parts->len);
    g_array_remove_range(compiler->triangles, 0, compiler->triangles->len);
    
    return mesh;
}

static RMeshGroup*
_output_meshgroup(
    OBJCompiler*    compiler
    )
{
    RMeshGroup* meshgroup;
    RMesh* mesh;
    gchar* object_name;
    GHashTableIter iter;
    
    meshgroup = r_meshgroup_new();
    g_hash_table_iter_init(&iter, compiler->objects);
    while(g_hash_table_iter_next(&iter, (gpointer)&object_name, (gpointer)&mesh))
    {
        g_hash_table_insert(meshgroup->groups, g_strdup(object_name), mesh);
    }
    return meshgroup;
}

static void
_object(
    OBJCompiler*    compiler
    )
{
    gchar* object_name = NULL;
    
    _expect(compiler, R_SYMBOL_STRING, &object_name);
    if(compiler->output)
    {
        _output_mesh(compiler);
    }
    compiler->current_object_name = g_strdup(object_name);
    compiler->output = TRUE;
}

static void
_part(
    OBJCompiler*    compiler
    )
{
    OBJPart part;
    gchar* material_name = NULL;
    
    _expect(compiler, R_SYMBOL_STRING, &material_name);
    part.material = r_resource_ref(material_name);
    part.offset = compiler->triangles->len * 3;
    part.count = 0;
    g_array_append_val(compiler->parts, part);
}

static void
_point(
    OBJCompiler*    compiler
    )
{
    float3 point;
    
    _expect(compiler, R_SYMBOL_FLOAT, &point.x);
    _expect(compiler, R_SYMBOL_FLOAT, &point.y);
    _expect(compiler, R_SYMBOL_FLOAT, &point.z);
    g_array_append_val(compiler->points, point);
}

static void
_normal(
    OBJCompiler*    compiler
    )
{
    float3 normal;
    
    _expect(compiler, R_SYMBOL_FLOAT, &normal.x);
    _expect(compiler, R_SYMBOL_FLOAT, &normal.y);
    _expect(compiler, R_SYMBOL_FLOAT, &normal.z);
    g_array_append_val(compiler->normals, normal);
}

static void
_texcoord(
    OBJCompiler*    compiler
    )
{
    float2 texcoord;
    
    _expect(compiler, R_SYMBOL_FLOAT, &texcoord.x);
    _expect(compiler, R_SYMBOL_FLOAT, &texcoord.y);
    g_array_append_val(compiler->texcoords, texcoord);
}

static void
_face(
    OBJCompiler*    compiler
    )
{
    OBJTriangle triangle;
    guint i;
    
    for(i = 0; i < 3; i++)
    {
        _expect(compiler, R_SYMBOL_INT, &triangle.index_point[i]);
        if(_accept(compiler, R_SYMBOL_NONE, NULL))
        {
            triangle.index_texcoord[i] = -1;
        }
        else
        {
            _expect(compiler, R_SYMBOL_INT, &triangle.index_texcoord[i]);
        }
        _expect(compiler, R_SYMBOL_INT, &triangle.index_normal[i]);
    }
    g_array_append_val(compiler->triangles, triangle);
    g_array_index(compiler->parts, OBJPart, compiler->parts->len - 1).count += 3;
    
}

static void
_block(
    OBJCompiler*    compiler
    )
{
    if(_accept(compiler, R_SYMBOL_IDENT, "o"))
    {
        _object(compiler);
        _expect(compiler, R_SYMBOL_EOL, NULL);
    }
    else if(_accept(compiler, R_SYMBOL_IDENT, "g"))
    {
        _part(compiler);
        _expect(compiler, R_SYMBOL_EOL, NULL);
    }
    else if(_accept(compiler, R_SYMBOL_IDENT, "v"))
    {
        _point(compiler);
        _expect(compiler, R_SYMBOL_EOL, NULL);
    }
    else if(_accept(compiler, R_SYMBOL_IDENT, "vn"))
    {
        _normal(compiler);
        _expect(compiler, R_SYMBOL_EOL, NULL);
    }
    else if(_accept(compiler, R_SYMBOL_IDENT, "vt"))
    {
        _texcoord(compiler);
        _expect(compiler, R_SYMBOL_EOL, NULL);
    }
    else if(_accept(compiler, R_SYMBOL_IDENT, "f"))
    {
        _face(compiler);
        _expect(compiler, R_SYMBOL_EOL, NULL);
    }
}

static gpointer
_load_from_file(
    const gchar*      file_name
    )
{
    FILE* stream;
    OBJCompiler compiler;
    gpointer result;
    
    stream = fopen (file_name, "rb");
    if(stream == NULL)
    {
        return NULL;
    }
    
    compiler.line = 0;
    compiler.pos = 0;
    compiler.token = NULL;
    compiler.buffer = NULL;
    compiler.points = g_array_new(FALSE, FALSE, sizeof(float3));
    compiler.texcoords = g_array_new(FALSE, FALSE, sizeof(float2));
    compiler.normals = g_array_new(FALSE, FALSE, sizeof(float3));
    compiler.parts = g_array_new(FALSE, FALSE, sizeof(OBJPart));
    compiler.triangles = g_array_new(FALSE, FALSE, sizeof(OBJTriangle));
    compiler.objects = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    compiler.output = FALSE;
     
    compiler.current_object_name = NULL;
    
    while(_get_symbol(&compiler, stream))
    {
        _block(&compiler);
    }
    _expect(&compiler, R_SYMBOL_EOF, NULL);
    
    result = _output_mesh(&compiler);
    if(g_hash_table_size(compiler.objects) >= 2)
    {
        result = _output_meshgroup(&compiler);
    }
     
    g_hash_table_destroy(compiler.objects);
    g_array_free(compiler.triangles, TRUE);
    g_array_free(compiler.parts, TRUE);
    g_array_free(compiler.texcoords, TRUE);
    g_array_free(compiler.normals, TRUE);
    g_array_free(compiler.points, TRUE);
    g_strfreev(compiler.buffer);
    fclose(stream);
    
    return result;
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
    return g_str_has_suffix(file_name, ".obj");
}

RModuleFactory obj_module_factory =
{
    "obj",
    "wavefront mesh loader",
    _get_instance,
    _accept_file
};

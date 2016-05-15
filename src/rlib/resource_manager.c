/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      resource_manager.c
 *
 *      Copyright 2009 Romuald Rousseau <romualdrousseau@gmail.com>
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

/* --- types --- */
typedef struct __ResourceManager    _ResourceManager;

/* --- structures --- */
struct __ResourceManager
{
/* private */
    GHashTable*     cache;
};

/* --- variables --- */
static _ResourceManager self = {NULL};

/* --- functions --- */
/*
 * _resource_manager_cache_value_new:
 *
 */
static RResourceManagerValue*
_resource_manager_cache_value_new(
    gpointer    key
    )
{
    RResourceManagerValue* value;
    
    value = g_slice_new0(RResourceManagerValue);
    value->user_ref = 0;
    value->type = R_RESOURCE_NONE;
    value->name = (gchar*)key;
    value->link = NULL;
    value->data = NULL;
    value->custom_free_func = NULL;
    r_game_signal_emit2("resource_manager_load", value);
    return value;
}

/*
 * _resource_manager_cache_value_destroy:
 *
 */
static void
_resource_manager_cache_value_destroy(
    gpointer    data
    )
{
    RResourceManagerValue* value = data;
    
    r_game_signal_emit2_with_default(
        "resource_manager_unload", 
        value, 
        (RGameCallback2) r_resource_default_unload
        );
    g_free(value->link);
    g_free(value->name);
    g_slice_free(RResourceManagerValue, value);
}

/*
 * _resource_remove_links:
 *
 */
static gboolean
_resource_remove_links(
    gpointer key,
    gpointer data,
    gpointer user_data
    )
{
    RResourceManagerValue* value = data;
    
    return (value->link != NULL) && g_str_equal(value->link, (gchar*) user_data);
}

/**
 * r_resource_manager_init:
 *
 **/
void
r_resource_manager_init()
{
    self.cache = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        _resource_manager_cache_value_destroy
        );
}

/**
 * r_resource_manager_destroy:
 *
 */
void
r_resource_manager_destroy()
{
    g_hash_table_destroy(self.cache);
}

/**
 * r_resource_manager_cleanup:
 *
 **/
void
r_resource_manager_cleanup()
{
    g_hash_table_remove_all(self.cache);
}

/**
 * r_resource_ref:
 *
 **/
gpointer
r_resource_ref(
    const gchar*      key
    )
{
    RResourceManagerValue* value;

    g_assert(key != NULL);

    value = (RResourceManagerValue*)g_hash_table_lookup(self.cache, key);
    if(value == NULL)
    {
        value = _resource_manager_cache_value_new((gpointer)g_strdup(key));
        g_hash_table_insert(self.cache, (gpointer)g_strdup(key), value);
    }
    if(value->link == NULL)
    {
        value->user_ref++;
    }
    return value->data;
}

/**
 * r_resource_unref:
 *
 **/
void
r_resource_unref(
    const gchar*      key
    )
{
    RResourceManagerValue* value;
    
    g_assert(key != NULL);

    value = (RResourceManagerValue*)g_hash_table_lookup(self.cache, key);
    if((value != NULL) && (value->link == NULL))
    {
        value->user_ref--;
        if(value->user_ref == 0)
        {
            g_hash_table_remove(self.cache, key);
            g_hash_table_foreach_remove(self.cache, _resource_remove_links, (gpointer)key);
        }
    }
}

/**
 * r_resource_default_unload:
 *
 **/
void
r_resource_link(
    RResourceManagerValue*  value,
    const gchar*            key
    )
{
    g_assert(value != NULL);
    
    g_free(value->link);
    value->link = g_strdup(key);
}

/**
 * r_resource_default_unload:
 *
 **/
void
r_resource_default_unload(
    RResourceManagerValue*  value
    )
{
    g_assert(value != NULL);
    
    switch(value->type)
    {
        case R_RESOURCE_MATERIAL:
            {
                RMaterial* material = value->data;
                r_material_free(material);
            }
            break;
        case R_RESOURCE_MESH:
            {
                RMesh* mesh = value->data;
                r_mesh_free(mesh);
            }
            break;
        case R_RESOURCE_MESHGROUP:
            {
                RMeshGroup* meshgroup = value->data;
                r_meshgroup_free(meshgroup);
            }
            break;
        case R_RESOURCE_SURFACE:
            {
                RSurface* surface = value->data;
                r_surface_free(surface);
            }
            break;
        case R_RESOURCE_FONT:
            {
                RFont* font = value->data;
                r_font_free(font);
            }
            break;
        case R_RESOURCE_CUSTOM:
            {
                if(value->custom_free_func != NULL)
                {
                    value->custom_free_func(value->data);
                } 
            }
            break;
    }
}

/**
 * r_resource_material_load:
 *
 **/
void
r_resource_material_load(
    RResourceManagerValue*  value,
    const gchar*            material_file_name
    )
{
    RMaterial* material;

    g_assert(value != NULL);

    if(material_file_name == NULL)
    {
        material = r_material_new();
    }
    else
    {
        material = r_material_new_from_file(material_file_name);
        if(material == NULL)
        {
            g_error("%s not found", material_file_name);
        }
    }
    
    value->type = R_RESOURCE_MATERIAL;
    value->data = material;
}

/**
 * r_resource_mesh_load:
 *
 **/
void
r_resource_mesh_load(
    RResourceManagerValue*  value,
    const gchar*            model_file_name,
    RMaterial*              skin
    )
{
    RMesh* mesh;
    
    g_assert(value != NULL);
    g_assert(model_file_name != NULL);
    
    mesh = r_mesh_new_from_file(model_file_name, skin);
    if(mesh == NULL)
    {
        g_error("%s not found", model_file_name);
    }
    
    value->type = R_RESOURCE_MESH;
    value->data = mesh;
}

/**
 * r_resource_mesh_load_multiple:
 *
 **/
void
r_resource_mesh_load_multiple(
    RResourceManagerValue*  value,
    const gchar**           model_file_names,
    RMaterial*              skin
    )
{
    RMesh* mesh;
    
    g_assert(value != NULL);
    g_assert(model_file_names != NULL);
    g_assert(model_file_names[0] != NULL);
    
    mesh = r_mesh_new_from_files(model_file_names, skin);
    if(mesh == NULL)
    {
        g_error("%s not found", model_file_names[0]);
    }
    
    value->type = R_RESOURCE_MESH;
    value->data = mesh;
}

/**
 * r_resource_meshgroup_load:
 *
 **/
void
r_resource_meshgroup_load(
    RResourceManagerValue*  value,
    const gchar*            model_file_name,
    RMaterial*              skin
    )
{
    RMeshGroup* meshgroup;
    
    g_assert(value != NULL);

    meshgroup = r_meshgroup_new_from_file(model_file_name, skin);
    if(meshgroup == NULL)
    {
        g_error("%s not found", model_file_name);
    }
    
    value->type = R_RESOURCE_MESHGROUP;
    value->data = meshgroup;
}

/**
 * r_resource_surface_load:
 *
 **/
void
r_resource_surface_load(
    RResourceManagerValue*  value,
    const gchar*            file_name,
    gfloat                  x,
    gfloat                  y,
    gfloat                  width,
    gfloat                  height
    )
{
    RSurface* surface;
    
    g_assert(value != NULL);
    
    surface = r_surface_new_from_file(file_name);
    if(surface == NULL)
    {
        g_error("%s not found", file_name);
    }
    surface->x = x;
    surface->y = y;
    surface->width = width;
    surface->height = height;
    
    value->type = R_RESOURCE_SURFACE;
    value->data  = surface;
}

/**
 * r_resource_font_load:
 *
 **/
void
r_resource_font_load(
    RResourceManagerValue*  value,
    const gchar*            file_name,
    guint                   char_width,
    guint                   char_height
    )
{
    RFont* font;
    
    g_assert(value != NULL);
    
    font = r_font_new_from_file(file_name);
    if(font == NULL)
    {
        g_error("%s not found", file_name);
    }
    font->char_width = char_width;
    font->char_height = char_height;
    
    value->type = R_RESOURCE_FONT;
    value->data  = font;
}

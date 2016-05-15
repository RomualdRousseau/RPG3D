/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      material.c
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

/* --- functions --- */
/**
 * r_material_new:
 *
 **/
RMaterial*
r_material_new()
{
    RMaterial* material;
    
    material = g_slice_new0(RMaterial);
    material->texture = R_TEXTURE_NONE;
    material->color.x = 1.0f;
    material->color.y = 1.0f;
    material->color.z = 1.0f;
    material->color.w = 1.0f;
    return material;
}

/**
 * r_material_new_from_file:
 *
 **/
RMaterial*
r_material_new_from_file(
    const gchar*        file_name
    )
{
    RMaterial* material;
    RImage* image;
    
    g_assert(file_name != NULL);
    
    image = r_image_new_from_file(file_name);
    if(image == NULL)
    {
        return NULL;
    }
    material = r_material_new();
    material->texture = r_texture_new(image, GL_LINEAR, GL_LINEAR, TRUE, TRUE);
    return material;
}

/**
 * r_material_free:
 *
 **/
void
r_material_free(
    RMaterial*          material
    )
{
    if(material != NULL)
    {
        r_texture_free(material->texture);
        g_slice_free(RMaterial, material);
    }
}

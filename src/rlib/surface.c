/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      surface.c
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

/* --- functions --- */
/**
 * r_surface_new:
 *
 **/
RSurface*
r_surface_new()
{
    RSurface* surface = g_slice_new0(RSurface);
    surface->texture = R_TEXTURE_NONE;
    surface->alpha = TRUE;
    surface->x = 0;
    surface->y = 0;
    surface->width = 1.0f;
    surface->height = 1.0f;
    return surface;
}

/**
 * r_surface_new_from_file:
 *
 **/
RSurface*
r_surface_new_from_file(
    const gchar*            file_name
    )
{
    RImage* image;
    RSurface* surface;
    
    g_assert(file_name != NULL);
    
    image = r_image_new_from_file(file_name);
    if(image == NULL)
    {
        return NULL;
    }
    surface = r_surface_new();
    surface->texture = r_texture_new(image, GL_LINEAR, GL_LINEAR, FALSE, TRUE);
    return surface;
}

/**
 * r_surface_free:
 *
 **/
extern void
r_surface_free(
    RSurface*               surface
)
{
    if(surface != NULL)
    {
        r_texture_free(surface->texture);
        g_slice_free(RSurface, surface);
    }
}

/**
 * r_surface_draw:
 *
 **/
void
r_surface_draw(
    RSurface*               surface,
    gint                    x,
    gint                    y,
    guint                   width,
    guint                   height
    )
{
    float2 vertice[4];
    float2 texcoords[4];

    g_assert(surface != NULL);

    vertice[0].x = x;
    vertice[0].y = 1000 - y;
    vertice[1].x = x + width;
    vertice[2].y = 1000 - y - height * surface->width / surface->height;
    vertice[1].y = vertice[0].y;
    vertice[2].x = vertice[1].x;
    vertice[3].x = vertice[0].x;
    vertice[3].y = vertice[2].y;

    texcoords[0].x = surface->x;
    texcoords[0].y = surface->y;
    texcoords[1].x = surface->x + surface->width;
    texcoords[2].y = surface->y + surface->height;
    texcoords[1].y = texcoords[0].y;
    texcoords[2].x = texcoords[1].x;
    texcoords[3].x = texcoords[0].x;
    texcoords[3].y = texcoords[2].y;
    
    if(surface->alpha)
    {
        glEnable(GL_BLEND);
    }
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, surface->texture);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, (gconstpointer)vertice);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, (gconstpointer)texcoords);

    glDrawArrays(GL_QUADS, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

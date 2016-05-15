/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      texture.c
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

/* --- types --- */
typedef struct __GenerateTextureParams _GenerateTextureParams;

typedef struct __ReplaceTextureParams _ReplaceTextureParams;

/* --- structures --- */
struct __GenerateTextureParams
{
    RImage*     image;
    gint        min_filter;
    gint        mag_filter;
    gboolean    wrap;
    gboolean    free_image;
};

struct __ReplaceTextureParams
{
    GLuint      texture;
    RImage*     image;
    gboolean    free_image;
};

/* --- functions --- */
/*
 * _texture_free_delegate:
 *
 */
static gpointer
_texture_free_delegate(
    gpointer    texture
    )
{
    glDeleteTextures(1, texture);
    return NULL;
}

/*
 * _texture_generate_delegate:
 *
 */
static gpointer
_texture_generate_delegate(
    _GenerateTextureParams*     params
    )
{
    GLuint texture = R_TEXTURE_NONE;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params->wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params->wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params->min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params->mag_filter);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    if(params->image->bytes_per_pixel == 3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, params->image->width, params->image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, params->image->pixel_data);
    }
    else if(params->image->bytes_per_pixel == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, params->image->width, params->image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, params->image->pixel_data);
    }
    
    if(params->free_image)
    {
        r_image_free(params->image);
    }

    g_free(params);
    return GUINT_TO_POINTER(texture);
}

/*
 * _texture_generate_mipmap_delegate:
 *
 */
static gpointer
_texture_generate_mipmap_delegate(
    _GenerateTextureParams*     params
    )
{
    GLuint texture = R_TEXTURE_NONE;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params->wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params->wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    
    if(params->image->bytes_per_pixel == 3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, params->image->width, params->image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, params->image->pixel_data);
    }
    else if(params->image->bytes_per_pixel == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, params->image->width, params->image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, params->image->pixel_data);
    }
    
    if(params->free_image)
    {
        r_image_free(params->image);
    }

    g_free(params);
    return GUINT_TO_POINTER(texture);
}

/*
 * _texture_replace_delegate:
 *
 */
static gpointer
_texture_replace_delegate(
    _ReplaceTextureParams*     params
    )
{
    glBindTexture(GL_TEXTURE_2D, params->texture);

    if(params->image->bytes_per_pixel == 3)
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, params->image->width, params->image->height, GL_RGB, GL_UNSIGNED_BYTE, params->image->pixel_data);
    }
    else if(params->image->bytes_per_pixel == 4)
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, params->image->width, params->image->height, GL_RGBA, GL_UNSIGNED_BYTE, params->image->pixel_data);
    }
    
    if(params->free_image)
    {
        r_image_free(params->image);
    }

    g_free(params);
    return NULL;
}

/**
 * r_texture_new:
 *
 **/
GLuint
r_texture_new(
    RImage*         image,
    gint            min_filter,
    gint            mag_filter,
    gboolean        wrap,
    gboolean        free_image
    )
{
    _GenerateTextureParams* params;

    g_assert(image != NULL);
    g_assert(image->bytes_per_pixel == 3 || image->bytes_per_pixel == 4);

    params = g_new0(_GenerateTextureParams, 1);
    params->image = image;
    params->min_filter = min_filter;
    params->mag_filter = mag_filter;
    params->wrap = wrap;
    params->free_image = free_image;

    return GPOINTER_TO_UINT(r_renderer_execute((GThreadFunc) _texture_generate_delegate, params));
}

/**
 * r_texture_new_mipmap:
 *
 **/
GLuint
r_texture_new_mipmap(
    RImage*         image,
    gint            min_filter,
    gint            mag_filter,
    gboolean        wrap,
    gboolean        free_image
    )
{
    _GenerateTextureParams* params;

    g_assert(image != NULL);
    g_assert(image->bytes_per_pixel == 3 || image->bytes_per_pixel == 4);

    params = g_new0(_GenerateTextureParams, 1);
    params->image = image;
    params->min_filter = min_filter;
    params->mag_filter = mag_filter;
    params->wrap = wrap;
    params->free_image = free_image;

    return GPOINTER_TO_UINT(r_renderer_execute((GThreadFunc) _texture_generate_mipmap_delegate, params));
}

/**
 * r_texture_free:
 *
 *
 **/
void
r_texture_free(
    GLuint      texture
    )
{
    if(texture != R_TEXTURE_NONE)
    {
        r_renderer_execute((GThreadFunc) _texture_free_delegate, &texture);
    }
}

/**
 * r_texture_replace:
 *
 *
 **/
void
r_texture_replace(
    GLuint          texture,
    RImage*         image,
    gboolean        free_image
    )
{
    _ReplaceTextureParams* params;

    g_assert(image != NULL);
    g_assert(image->bytes_per_pixel == 3 || image->bytes_per_pixel == 4);
    g_assert(texture != R_TEXTURE_NONE);
    
    params = g_new0(_ReplaceTextureParams, 1);
    params->texture = texture;
    params->image = image;
    params->free_image = free_image;

    r_renderer_execute((GThreadFunc) _texture_replace_delegate, params);
}


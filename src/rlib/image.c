/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      image.c
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

/* --- functions --- */
/**
 * r_image_new
 * 
 **/
RImage*
r_image_new(
    guint           width,
    guint           height,
    guint           bytes_per_pixel
    )
{
    RImage* image;
    
    image = g_slice_new0(RImage);
    image->width = width;
    image->height = height;
    image->bytes_per_pixel = bytes_per_pixel;
    image->pixel_data = g_new0(guint8, width * height * bytes_per_pixel); 
    return image;
}

/**
 * r_image_new_from_file:
 *
 **/
RImage*
r_image_new_from_file(
    const gchar*      file_name
    )
{
    g_assert(file_name != NULL);

    return (RImage*) r_modules_lookup(file_name)->load_from_file(file_name);
}

/**
 * r_image_free:
 *
 **/
void
r_image_free(
    RImage*     image
    )
{
    g_assert(image != NULL);

    g_free(image->pixel_data);
    g_slice_free(RImage, image);
}

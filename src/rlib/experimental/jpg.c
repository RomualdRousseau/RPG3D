/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      jpg.c
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

#include <globals.h>

#include<stdio.h>
#include<string.h>
#include<jpeglib.h>

static gpointer
_load_from_file(
    gchar* file_name
    )
{
    FILE* stream;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPARRAY buffer;
    Image* image;
    gint i;
    gint j;
    guint8* p;
    guint8* k;

    stream = fopen(file_name, "rb");
    if(stream == NULL)
    {
        return NULL;
    }

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, stream);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
    g_assert(
        (cinfo.output_components == 3) ||
        (cinfo.output_components == 4));

    image = image_new(cinfo.output_width, cinfo.output_height, 3);

    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, cinfo.output_width * cinfo.output_components, 1);

    for(;;)
    {
        j = cinfo.output_scanline;
        if(j >= cinfo.output_height)
        {
            break;
        }
        jpeg_read_scanlines(&cinfo, buffer, 1);
        for(i = 0; i < cinfo.output_width; i++)
        {
            p = &image->pixel_data[(cinfo.output_width * j + i) * cinfo.output_components];
            k = &buffer[0][i * cinfo.output_components];
            memcpy(p, k, cinfo.output_components);
        }
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(stream);

    return (gpointer) image;
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
    gchar*      file_name
    )
{
    return g_str_has_suffix(file_name, ".jpg");
}

RModuleFactory jpg_module_factory =
{
    "jpg",
    "jpeg image loader",
    _get_instance,
    _accept_file
};

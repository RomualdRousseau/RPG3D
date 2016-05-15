/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      tga.c
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
#include <string.h>

struct _TGAHeader
{
    gchar       idlength;
    gchar       colourmaptype;
    gchar       datatypecode;
    gshort      colourmaporigin;
    gshort      colourmaplength;
    gchar       colourmapdepth;
    gshort      x_origin;
    gshort      y_origin;
    gshort      width;
    gshort      height;
    gchar       bitsperpixel;
    gchar       imagedescriptor;
} __attribute__((__packed__));
typedef struct _TGAHeader TGAHeader;

static void
_fread_flat(
    RImage*         image,
    FILE*           stream
)
{
    fread(image->pixel_data, image->width * image->height * image->bytes_per_pixel, 1, stream);
}

static void
_fread_rle(
    RImage*         image,
    FILE*           stream
)
{
    gint i;
    guint8 b;
    guint8* p;
    
    p = &image->pixel_data[0];
    while(p < &image->pixel_data[image->width * image->height * image->bytes_per_pixel])
    {
        fread(&b, 1, 1, stream);
        if(b & 0x80)
        {
            fread(p, image->bytes_per_pixel, 1, stream);
            p += image->bytes_per_pixel;
            for(i = b & 0x7F; i > 0; i--)
            {
                memcpy(p, p - image->bytes_per_pixel, image->bytes_per_pixel);
                p += image->bytes_per_pixel;
            }
        }
        else
        {
            for(i = b + 1; i > 0; i--)
            {
                fread(p, image->bytes_per_pixel, 1, stream);
                p += image->bytes_per_pixel;
            }
        }
    }
}

static void
_mirror_horizontal(
    RImage*         image
)
{
    guint i, j;
    guint8 swap[4];
    guint8* row1;
    guint8* row2;
    
    for(i = 0; i < image->height; i++)
    {
        row1 = &image->pixel_data[image->bytes_per_pixel * image->width * i];
        row2 = &image->pixel_data[image->bytes_per_pixel * (image->width * (i + 1) - 1)];
        for(j = 0; j < image->width / 2; j++)
        {
            memcpy(swap, row1, image->bytes_per_pixel);
            memcpy(row1, row2, image->bytes_per_pixel);
            memcpy(row2, swap, image->bytes_per_pixel);
            row1 += image->bytes_per_pixel;
            row2 -= image->bytes_per_pixel;
        }
    }
}

static void
_mirror_vertical(
    RImage*         image
)
{
    guint i, j;
    guint8 swap[4];
    guint8* row1;
    guint8* row2;
    
    for(i = 0; i < image->height / 2; i++)
    {
        row1 = &image->pixel_data[image->bytes_per_pixel * image->width * i];
        row2 = &image->pixel_data[image->bytes_per_pixel * image->width * (image->height - 1 - i)];
        for(j = 0; j < image->width; j++)
        {
            memcpy(swap, row1, image->bytes_per_pixel);
            memcpy(row1, row2, image->bytes_per_pixel);
            memcpy(row2, swap, image->bytes_per_pixel);
            row1 += image->bytes_per_pixel;
            row2 += image->bytes_per_pixel;
        }
    }
}

static void
_convert_rgba(
    RImage*         image
)
{
    guint8* p;
    
    p = &image->pixel_data[0];
    while(p < &image->pixel_data[image->width * image->height * image->bytes_per_pixel])
    {
        p[0] ^= p[2];
        p[2] ^= p[0];
        p[0] ^= p[2];
        p += image->bytes_per_pixel;
    }
}

static gpointer
_load_from_file(
    const gchar*      file_name
    )
{
    RImage* image;
    FILE* stream;
    TGAHeader header;
    
    stream = fopen(file_name, "rb");
    if(stream == NULL)
    {
        return NULL;
    }
    fread(&header, sizeof(TGAHeader), 1, stream);
    fseek(stream, header.idlength, SEEK_CUR);
    
    g_assert(
        (header.colourmaptype == 0 && header.bitsperpixel == 24) ||
        (header.colourmaptype == 0 && header.bitsperpixel == 32));
    
    image = r_image_new(header.width, header.height, header.bitsperpixel / 8);
    
    if(header.datatypecode == 2)
    {
        _fread_flat(image, stream);
    }
    else
    {
        _fread_rle(image, stream);
    }
    if(header.imagedescriptor & 16)
    {
        _mirror_horizontal(image);
    }
    if(!(header.imagedescriptor & 32))
    {
        _mirror_vertical(image);
    }
    _convert_rgba(image);
    
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
    const gchar*      file_name
    )
{
    return g_str_has_suffix(file_name, ".tga");
}

RModuleFactory tga_module_factory =
{
    "tga",
    "tga image loader",
    _get_instance,
    _accept_file
};

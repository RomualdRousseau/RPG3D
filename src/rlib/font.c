/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      renderer.c
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
 * r_font_new:
 *
 **/
RFont*
r_font_new()
{
    RFont* font = g_slice_new0(RFont);
    font->texture = R_TEXTURE_NONE;
    font->alpha = TRUE;
    font->char_width = 0.0f;
    font->char_height = 0.0f;
    return font;
}

/**
 * r_font_new_from_file:
 *
 **/
RFont*
r_font_new_from_file(
    const gchar*            file_name
    )
{
    RImage* image;
    RFont* font;
    
    g_assert(file_name != NULL);
    
    image = r_image_new_from_file(file_name);
    if(image == NULL)
    {
        return NULL;
    }
    font = r_font_new();
    font->texture = r_texture_new(image, GL_LINEAR, GL_LINEAR, FALSE, TRUE);
    return font;
}

/**
 * r_font_free:
 *
 **/
extern void
r_font_free(
    RFont*                  font
)
{
    if(font != NULL)
    {
        r_texture_free(font->texture);
        g_slice_free(RFont, font);
    }
}

/**
 * r_font_draw_char:
 *
 **/
void
r_font_draw_char(
    RFont*          font,
    guint           font_size,
    gint            x,
    gint            y,
    const gchar     cc
    )
{
    float2 vertice[4];
    float2 texcoords[4];
    gfloat cx, cy;
    gfloat cw, ch;
    gchar c;
    
    g_assert(font != NULL);
    
    c = cc - ' ';
    cx = c % font->char_width;
    cy = c / font->char_width;
    cw = 1.0f / (gfloat) font->char_width;
    ch = 1.0f / (gfloat) font->char_height;
    
    vertice[0].x = x;
    vertice[0].y = 1000 - y;
    vertice[1].x = x + font_size;
    vertice[1].y = vertice[0].y;
    vertice[2].x = vertice[1].x;
    vertice[2].y = 1000 - y - font_size * font->char_width / font->char_height;
    vertice[3].x = vertice[0].x;
    vertice[3].y = vertice[2].y;

    texcoords[0].x = cx * cw;
    texcoords[0].y = cy * ch;
    texcoords[1].x = (cx + 1.0f - cw) * cw;
    texcoords[1].y = texcoords[0].y;
    texcoords[2].x = texcoords[1].x;
    texcoords[2].y = (cy + 1.0f - ch) * ch;
    texcoords[3].x = texcoords[0].x;
    texcoords[3].y = texcoords[2].y;
    
    if(font->alpha)
    {
        glEnable(GL_BLEND);
    }
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font->texture);
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

/**
 * r_font_draw_string:
 *
 **/
void
r_font_draw_string(
    RFont*          font,
    guint           font_size,
    gint            x,
    gint            y,
    const gchar*    s
    )
{
    float2 vertice[4];
    float2 texcoords[4];
    gfloat cx, cy;
    gfloat cw, ch;
    gchar c;
    
    g_assert(font != NULL);
    g_assert(s != NULL);
    
    cw = 1.0f / (gfloat) font->char_width;
    ch = 1.0f / (gfloat) font->char_height;

    if(font->alpha)
    {
        glEnable(GL_BLEND);
    }
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font->texture);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, (gconstpointer)vertice);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, (gconstpointer)texcoords);

    while(*s)
    {
        c = *s - ' ';
        cy = c / font->char_width;
        cx = c - cy * font->char_width;

        vertice[0].x = x;
        vertice[0].y = 1000 - y;
        vertice[1].x = x + font_size;
        vertice[1].y = vertice[0].y;
        vertice[2].x = vertice[1].x;
        vertice[2].y = 1000 - y - font_size;
        vertice[3].x = vertice[0].x;
        vertice[3].y = vertice[2].y;

        texcoords[0].x = cx * cw;
        texcoords[0].y = cy * ch;
        texcoords[1].x = (cx + 1.0f - cw) * cw;
        texcoords[1].y = texcoords[0].y;
        texcoords[2].x = texcoords[1].x;
        texcoords[2].y = (cy + 1.0f - ch) * ch;
        texcoords[3].x = texcoords[0].x;
        texcoords[3].y = texcoords[2].y;

        glDrawArrays(GL_QUADS, 0, 4);

        x += font_size;
        s++;
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

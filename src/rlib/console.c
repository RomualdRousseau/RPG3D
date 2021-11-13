/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      console.c
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
#include <string.h>
#include <ctype.h>

#define BUFFER_WIDTH    80
#define BUFFER_HEIGHT   25

/* --- types --- */
typedef struct __Console _Console;

/* --- structures --- */
struct __Console
{
    GMutex       console_lock;
    gchar*       text_buffer;
    GString*     input_buffer;
    gint         cursor_x;
    gint         cursor_y;
};

/* --- variables --- */
static _Console self = {{0}, NULL, NULL, 0, 0};


/* --- functions --- */
/*
 * _console_text_changed_default:
 *
 */
static void
_console_text_changed_default(
    gpointer    text
    )
{
    RGameCallback2 callback;
    gchar** args;
    gchar* command;
    
    args = g_strsplit((gchar*)text, " ", 0);
    if(args[0] != NULL)
    {
        command = g_strconcat("console_", args[0], NULL);
        callback = (RGameCallback2) r_game_signal_get_address(command);
        g_free(command);
        
        if(callback != NULL)
        {
            callback(args);
        }
        else
        {
            r_console_printf("%s: command not found\n", args[0]);
        }
    }
    g_strfreev(args); 
    
    r_console_print(R_CONSOLE_PROMPT);
}

/*
 * _console_cursor_move_next_line:
 *
 */
static void
_console_cursor_move_next_line()
{
    self.cursor_x = 0;
    self.cursor_y++;
    if(self.cursor_y >= BUFFER_HEIGHT)
    {
        self.cursor_y = BUFFER_HEIGHT - 1;
        memmove(&self.text_buffer[0], &self.text_buffer[BUFFER_WIDTH], BUFFER_HEIGHT * BUFFER_WIDTH);
    }
}

/*
 * _console_cursor_move_prev_line:
 *
 */
static void
_console_cursor_move_prev_line()
{
    self.cursor_x = BUFFER_WIDTH - 1;
    self.cursor_y--;
    if(self.cursor_y < 0)
    {
        self.cursor_y = 0;
    }
}

/*
 * _console_cursor_move_next:
 *
 */
static void
_console_cursor_move_next()
{
    self.cursor_x++;
    if(self.cursor_x >= BUFFER_WIDTH)
    {
        _console_cursor_move_next_line();
    }
}

/*
 * _console_cursor_move_prev:
 *
 */
static void
_console_cursor_move_prev()
{
    self.cursor_x--;
    if(self.cursor_x < 0)
    {
        _console_cursor_move_prev_line();
    }
}

/*
 * _console_putchar:
 *
 */
static void
_console_putchar(const gchar c)
{
    if(c == '\b')
    {
        _console_cursor_move_prev();
        self.text_buffer[self.cursor_y * BUFFER_WIDTH + self.cursor_x] = '\0';
    }
    else if(c == '\n')
    {
        _console_cursor_move_next_line();
    }
    else
    {
        self.text_buffer[self.cursor_y * BUFFER_WIDTH + self.cursor_x] = c;
        _console_cursor_move_next();
    }
}

/*
 * _console_key:
 *
 */
void
_console_key(
    XKeyEvent*      key_event
    )
{
    gchar c;
    KeySym k;

    if (XLookupString(key_event, &c, 1, &k, NULL) == 1)
    {
        if((k == XK_Return) || (k == XK_KP_Enter))
        {
            g_mutex_lock(&self.console_lock);
            _console_putchar('\n');
            g_mutex_unlock(&self.console_lock);
            r_game_signal_emit2_with_default(
                "console_text_changed",
                self.input_buffer->str,
                _console_text_changed_default
                );
            g_string_erase(self.input_buffer, 0, -1);
        }
        else if(k == XK_BackSpace)
        {
            if(self.input_buffer->len > 0)
            {
                g_mutex_lock(&self.console_lock);
                _console_putchar('\b');
                g_mutex_unlock(&self.console_lock);
                g_string_erase(self.input_buffer, self.input_buffer->len - 1, 1);
            }
        }
        else if(isprint(c))
        {
            g_mutex_lock(&self.console_lock);
            _console_putchar(c);
            g_mutex_unlock(&self.console_lock);
            g_string_append_c(self.input_buffer, c);
        }
    }
}

/**
 * r_console_init:
 *
 **/
void
r_console_init()
{
    g_mutex_init(&self.console_lock);
    self.text_buffer = g_new0(gchar, BUFFER_WIDTH * (BUFFER_HEIGHT + 1));
    self.input_buffer = g_string_new("");
    self.cursor_x = 0;
    self.cursor_y = 0;
    r_game_signal_connect("game_key", (RGameCallback) _console_key);
}

/**
 * r_console_destroy:
 *
 **/
void
r_console_destroy()
{
    g_string_free(self.input_buffer, TRUE);
    g_free(self.text_buffer);
    g_mutex_clear(&self.console_lock);
}

/**
 * r_console_goto:
 * @x:
 * @y:
 *
 **/
void
r_console_goto(
    guint           x,
    guint           y
    )
{
    g_mutex_lock(&self.console_lock);
    self.cursor_x = x;
    self.cursor_y = y;
    g_mutex_unlock(&self.console_lock);
}

/**
 * r_console_putchar:
 * @c:
 *
 **/
void
r_console_putchar(
    const gchar     c
    )
{
    g_mutex_lock(&self.console_lock);
    _console_putchar(c);
    g_mutex_unlock(&self.console_lock);
}

/**
 * r_console_print:
 * @s:
 *
 **/
void
r_console_print(
    const gchar*    s
    )
{
    g_assert(s != NULL);

    g_mutex_lock(&self.console_lock);
    while(*s)
    {
        _console_putchar(*s++);
    }
    g_mutex_unlock(&self.console_lock);
}

/**
 * r_console_printf:
 * @format:
 *
 **/
void
r_console_printf(
    const gchar*        format,
    ...
    )
{
    va_list args;
    gchar* s;
    
    va_start(args, format);
    s = g_strdup_vprintf(format, args);
    r_console_print(s);
    g_free(s);
    va_end(args);
}


/**
 * r_console_draw:
 * @font:
 * @x:
 * @y,
 * @width:
 * @height
 *
 **/
void
r_console_draw(
    RFont*      font,
    gint        x,
    gint        y,
    guint       width,
    guint       height
    )
{
    float2 vertice[4];
    float2 texcoords[4];
    guint ix, iy;
    gfloat cx, cy;
    gfloat cw, ch;
    gchar c;
    guint i, j;

    g_assert(font != NULL);

    if(font->char_height == 0)
    {
        return;
    }

    ix = width / BUFFER_WIDTH;
    iy = height / BUFFER_HEIGHT;
    
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
    
    for(i = 0; i < BUFFER_HEIGHT; i++)
    {
        for(j = 0; j < BUFFER_WIDTH; j++)
        {
            c = self.text_buffer[i * BUFFER_WIDTH + j];
            if(c != '\0')
            {
                c -= ' ';
                cx = c % font->char_width;
                cy = c / font->char_width;

                vertice[0].x = x + ix * j;
                vertice[0].y = 1000 - y - iy * i;
                vertice[1].x = x + ix * (j + 1);
                vertice[1].y = vertice[0].y;
                vertice[2].x = vertice[1].x;
                vertice[2].y = 1000 - y - iy * (i + 1);
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
            }
        }
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}




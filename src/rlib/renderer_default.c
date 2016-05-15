/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      renderer_default.c
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

/* --- functions --- */
/*
 * _renderer_default_init:
 *
 */
static void
_renderer_default_init()
{
}

/*
 * _renderer_default_destroy:
 *
 */
static void
_renderer_default_destroy()
{
}

/*
 * _renderer_default_update:
 *
 */
static void
_renderer_default_update()
{
    r_renderer_render_scene();
    r_renderer_swap_buffers();
}

/*
 * _renderer_default_pause:
 *
 */
static void
_renderer_default_pause()
{
}

/*
 * _renderer_default_resume:
 *
 */
static void
_renderer_default_resume()
{
}

/*
 * _renderer_default_resize:
 *
 */
static void
_renderer_default_resize(
    guint           width,
    guint           height
    )
{
    r_renderer_resize_viewport(width, height);
}

/*
 * _renderer_default_execute:
 *
 */
static gpointer
_renderer_default_execute(
    GThreadFunc     function,
    gpointer        user_data,
    GSourceFunc     completed_function
    )
{
    gpointer return_value;

    g_assert(function != NULL);

    return_value = function(user_data);
    if(completed_function != NULL)
    {
        g_idle_add(completed_function, user_data);
    }
    return return_value;
}

/*
 * _create_instance:
 *
 */
static RRenderer
_create_instance()
{
    RRenderer singleton = renderer;
    singleton->init = _renderer_default_init;
    singleton->destroy = _renderer_default_destroy;
    singleton->update = _renderer_default_update;
    singleton->pause = _renderer_default_pause;
    singleton->resume = _renderer_default_resume;
    singleton->resize = _renderer_default_resize;
    singleton->execute = _renderer_default_execute;
    return singleton;
}

/*
 * renderer_default_factory:
 *
 */
RRendererFactory renderer_default_factory =
{
    "RendererDefault",
    "default renderer",
    _create_instance
};

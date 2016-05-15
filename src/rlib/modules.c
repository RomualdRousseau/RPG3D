/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      modules.c
 *
 *      Copyright 2009 Romuald Rousseau <romualdrousseau@msn.com>
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

/* --- variables --- */
extern RModuleFactory tga_module_factory;
extern RModuleFactory md2_module_factory;
extern RModuleFactory obj_module_factory;

static RModuleFactory* module_factories[] =
{
    &tga_module_factory,
    &md2_module_factory,
    &obj_module_factory,
    NULL
};

/* --- functions --- */
/*
 * _modules_get_factory:
 *
 */
static RModuleFactory*
_modules_get_factory(
    const gchar*      file_name
    )
{
    RModuleFactory* module_factory = NULL;;
    gchar* file_name_down;
    gint i;

    file_name_down = g_ascii_strdown(file_name, -1);
    for(i = 0; module_factories[i] != NULL; i++)
    {
        if(module_factories[i]->accept_file(file_name_down))
        {
            module_factory = module_factories[i];
            break;
        }
    }
    g_free(file_name_down);
    return module_factory;
}

/**
 * r_modules_init:
 *
 **/
void
r_modules_init()
{
    gint i;

    g_message("Loading modules...");

    for(i = 0; module_factories[i] != NULL; i++)
    {
        g_message(" %s: %s", module_factories[i]->name, module_factories[i]->description);
    }
}

/**
 * r_modules_destroy:
 *
 **/
void
r_modules_destroy()
{
}

/**
 * r_modules_lookup:
 *
 **/
RModule*
r_modules_lookup(
    const gchar*      file_name
    )
{
    RModuleFactory* module_factory = NULL;;

    g_assert(file_name != NULL);

    module_factory = _modules_get_factory(file_name);
    if(module_factory == NULL)
    {
        g_error("No module found for '%s', be sure to compile the rlib with the required support.", file_name);
    }

#if DEBUG
    g_debug("Use module: %s: %s", module_factory->name, module_factory->description);
#endif

    return module_factory->get_instance();
}


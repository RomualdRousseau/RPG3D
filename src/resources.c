/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      resources.c
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

#include <globals.h>

    const gchar* hero1_file_names[32] = 
    {
        // stand
        PACKAGE_DATADIR "/alien/alien_000000.obj",
        PACKAGE_DATADIR "/alien/alien_000004.obj",
        PACKAGE_DATADIR "/alien/alien_000009.obj",
        PACKAGE_DATADIR "/alien/alien_000014.obj",
        PACKAGE_DATADIR "/alien/alien_000019.obj",
        PACKAGE_DATADIR "/alien/alien_000024.obj",
        PACKAGE_DATADIR "/alien/alien_000029.obj",
        PACKAGE_DATADIR "/alien/alien_000034.obj",
        PACKAGE_DATADIR "/alien/alien_000039.obj",
        PACKAGE_DATADIR "/alien/alien_000044.obj",
        PACKAGE_DATADIR "/alien/alien_000049.obj",
        PACKAGE_DATADIR "/alien/alien_000054.obj",
        PACKAGE_DATADIR "/alien/alien_000059.obj",
        PACKAGE_DATADIR "/alien/alien_000064.obj",
        PACKAGE_DATADIR "/alien/alien_000069.obj",
        PACKAGE_DATADIR "/alien/alien_000074.obj",
        PACKAGE_DATADIR "/alien/alien_000078.obj",
        // run
        PACKAGE_DATADIR "/alien/alien_000079.obj",
        PACKAGE_DATADIR "/alien/alien_000084.obj",
        PACKAGE_DATADIR "/alien/alien_000089.obj",
        PACKAGE_DATADIR "/alien/alien_000094.obj",
        PACKAGE_DATADIR "/alien/alien_000097.obj",
        // jump
        PACKAGE_DATADIR "/alien/alien_000098.obj",
        PACKAGE_DATADIR "/alien/alien_000103.obj",
        PACKAGE_DATADIR "/alien/alien_000108.obj",
        PACKAGE_DATADIR "/alien/alien_000113.obj",
        PACKAGE_DATADIR "/alien/alien_000118.obj",
        PACKAGE_DATADIR "/alien/alien_000123.obj",
        PACKAGE_DATADIR "/alien/alien_000128.obj",
        PACKAGE_DATADIR "/alien/alien_000133.obj",
        PACKAGE_DATADIR "/alien/alien_000136.obj",
        NULL
    };

/* --- functions --- */
void
resources_load_manor(
    RResourceManagerValue* value
    )
{
    if(g_str_equal(value->name, "meshes.manor"))
    {
        value->type = R_RESOURCE_CUSTOM;
        value->data  = world_new(r_resource_ref("meshes.manor.data"));
        value->custom_free_func = (GDestroyNotify)world_free;
    } 
    else if(g_str_equal(value->name, "meshes.manor.data"))
    {
        r_resource_meshgroup_load(
            value,
            PACKAGE_DATADIR "/manor.obj",
            NULL
            );
        r_resource_link(value, "meshes.manor");
    }
    else if(g_str_equal(value->name, "meshes.manor.none"))
    {
        r_resource_material_load(
            value,
            NULL
            );
        r_resource_link(value, "meshes.manor");
    }
    else if(g_str_equal(value->name, "meshes.manor.ceramic"))
    {
        r_resource_material_load(
            value,
            PACKAGE_DATADIR "/ceramic.tga"
            );
        r_resource_link(value, "meshes.manor");
    }
    else if(g_str_equal(value->name, "meshes.manor.stone"))
    {
        r_resource_material_load(
            value,
            PACKAGE_DATADIR "/stone.tga"
            );
        r_resource_link(value, "meshes.manor");
    }
    else if(g_str_equal(value->name, "meshes.manor.wood"))
    {
        r_resource_material_load(
            value,
            PACKAGE_DATADIR "/wood.tga"
            );
        r_resource_link(value, "meshes.manor");
    }
    else if(g_str_equal(value->name, "meshes.manor.wall"))
    {
        r_resource_material_load(
            value,
            PACKAGE_DATADIR "/wall.tga"
            );
        r_resource_link(value, "meshes.manor");
    }
    else if(g_str_equal(value->name, "meshes.manor.brick"))
    {
        r_resource_material_load(
            value,
            PACKAGE_DATADIR "/brick.tga"
            );
        r_resource_link(value, "meshes.manor");
    }
    else if(g_str_equal(value->name, "meshes.manor.roof"))
    {
        r_resource_material_load(
            value,
            PACKAGE_DATADIR "/roof.tga"
            );
        r_resource_link(value, "meshes.manor");
    }
    else if(g_str_equal(value->name, "meshes.manor.concreate"))
    {
        r_resource_material_load(
            value,
            PACKAGE_DATADIR "/concreate.tga"
            );
        r_resource_link(value, "meshes.manor");
    }
}

void
resources_load_heroes(
    RResourceManagerValue* value
    )
{
    if(g_str_equal(value->name, "meshes.hero1"))
    {
        r_resource_mesh_load(
            value,
            PACKAGE_DATADIR "/blade.md2",
            r_resource_ref("meshes.hero1.skin")
            );
/*
        r_resource_mesh_load_multiple(
            value,
            hero1_file_names,
            r_resource_ref("meshes.hero1.skin")
            );
*/
    }
    else if(g_str_equal(value->name, "meshes.hero1.skin"))
    {
        r_resource_material_load(
            value,
            PACKAGE_DATADIR "/blade.tga"
            );
/*
        r_resource_material_load(
            value,
            NULL
            );
        r_resource_link(value, "meshes.hero1");
*/
    }
    else if(g_str_equal(value->name, "meshes.hero2"))
    {
        r_resource_mesh_load(
            value,
            PACKAGE_DATADIR "/ladydeath.md2",
            r_resource_ref("meshes.hero2.skin")
            );
    }
    else if(g_str_equal(value->name, "meshes.hero2.skin"))
    {
        r_resource_material_load(
            value,
            PACKAGE_DATADIR "/ladydeath.tga"
            );
        r_resource_link(value, "meshes.hero2");
    }
    else if(g_str_equal(value->name, "meshes.hero3"))
    {
        r_resource_mesh_load(
            value,
            PACKAGE_DATADIR "/warrior.md2",
            r_resource_ref("meshes.hero3.skin")
            );
    }
    else if(g_str_equal(value->name, "meshes.hero3.skin"))
    {
        r_resource_material_load(
            value,
            PACKAGE_DATADIR "/warrior.tga"
            );
        r_resource_link(value, "meshes.hero3");
    }
    else if(g_str_equal(value->name, "meshes.hero4"))
    {
        r_resource_mesh_load(
            value,
            PACKAGE_DATADIR "/yoko.md2",
            r_resource_ref("meshes.hero4.skin")
            );
    }
    else if(g_str_equal(value->name, "meshes.hero4.skin"))
    {
        r_resource_material_load(
            value,
            PACKAGE_DATADIR "/yoko.tga"
            );
        r_resource_link(value, "meshes.hero4");
    }
    else if(g_str_equal(value->name, "meshes.hero5"))
    {
        r_resource_mesh_load(
            value,
            PACKAGE_DATADIR "/hueteotl.md2",
            r_resource_ref("meshes.hero5.skin")
            );
    }
    else if(g_str_equal(value->name, "meshes.hero5.skin"))
    {
        r_resource_material_load(
            value,
            PACKAGE_DATADIR "/hueteotl.tga"
            );
        r_resource_link(value, "meshes.hero5");
    }
    else if(g_str_equal(value->name, "meshes.hero6"))
    {
        r_resource_mesh_load(
            value,
            PACKAGE_DATADIR "/slith.md2",
            r_resource_ref("meshes.hero6.skin")
            );
    }
    else if(g_str_equal(value->name, "meshes.hero6.skin"))
    {
        r_resource_material_load(
            value,
            PACKAGE_DATADIR "/slith.tga"
            );
        r_resource_link(value, "meshes.hero6");
    }
    else if(g_str_equal(value->name, "meshes.hero7"))
    {
        r_resource_mesh_load(
            value,
            PACKAGE_DATADIR "/rhino.md2",
            r_resource_ref("meshes.hero7.skin")
            );
    }
    else if(g_str_equal(value->name, "meshes.hero7.skin"))
    {
        r_resource_material_load(
            value,
            PACKAGE_DATADIR "/rhino.tga"
            );
        r_resource_link(value, "meshes.hero7");
    }
}

void
resources_load_console(
    RResourceManagerValue* value
    )
{
    if(g_str_equal(value->name, "console.hud"))
    {
        r_resource_surface_load(
            value,
            PACKAGE_DATADIR "/hud.tga",
            0.0f/256.0f, 0.0f/256.0f, 256.0f/256.0f, 256.0f/256.0f
            );
    }
    else if(g_str_equal(value->name, "console.font"))
    {
        r_resource_font_load(
            value,
            PACKAGE_DATADIR "/megatron.tga",
            512.0f / 32.0f, 256.0f / 42.0f
            );
    }
}

void
resources_load(
    RResourceManagerValue* value
    )
{
    resources_load_manor(value);
    resources_load_heroes(value);
    resources_load_console(value);
}

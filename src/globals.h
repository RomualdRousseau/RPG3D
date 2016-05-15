/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      globals.h
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

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <rlib.h>

#define TITLE               PACKAGE_STRING
#define WIDTH               1024
#define HEIGHT              768

/* --- types --- */
enum
{
    GAME_INIT,
    GAME_DESTROY,
    GAME_LOADING,
    GAME_SCENE,
    GAME_CHANGE_HERO
};

enum
{
    ACTION_QUIT,
    ACTION_FULLSCREEN_TOGGLE,
    ACTION_GRAB_INPUT_TOGGLE,
    ACTION_CONSOLE_MODE_TOGGLE,
    ACTION_CHANGE_HERO,
    ACTION_HERO_FORWARD,
    ACTION_HERO_BACKWARD,
    ACTION_HERO_LEFT,
    ACTION_HERO_RIGHT,
    ACTION_HERO_JUMP,
    ACTION_HERO_CROUCH
};

enum
{
    ENTITY_ACTION_NONE,
    ENTITY_ACTION_RUNNING,
    ENTITY_ACTION_JUMPING,
    ENTITY_ACTION_FALLING
};

enum
{
    WORLD_ROOM,
    WORLD_PORTAL,
    WORLD_SCULTURE
};

/* --- structures --- */
struct _Kernel
{
    guint        state;
    guint        frame_count;
    gboolean     fullscreen;
    gboolean     grab_input;
    guint        selected_hero;
    guint        desired_hero;
    gshort*      actions[16];
};
typedef struct _Kernel Kernel;

struct _Console
{
    RSurface*    hud;
    RFont*       font;
    gboolean     mode;
};
typedef struct _Console Console;

union _WorldNode
{
    struct
    {
        guint               id;
        guint               type;
        gboolean            visited;
        RMesh*              mesh;
        float3              bbox[2];
    }
    any;
    
    struct
    {
        guint               id;
        guint               type;
        gboolean            visited;
        RMesh*              mesh;
        float3              bbox[2];
        GList*              portals;
        GList*              scultures;
    }
    room;

    struct 
    {
        guint               id;
        guint               type;
        gboolean            visited;
        RMesh*              mesh;
        float3              bbox[2];
        union _WorldNode*   front;
        union _WorldNode*   back;
    }
    portal;

    struct
    {
        guint               id;
        guint               type;
        gboolean            visited;
        RMesh*              mesh;
        float3              bbox[2];
        union _WorldNode*   owner;
    }
    sculture;
};
typedef union _WorldNode WorldNode;

struct _World
{
    float3      camera_position;
    gfloat      camera_rotation;
    GArray*     nodes;
    RMeshGroup* groups;
};
typedef struct _World World;

struct _Entity
{
    float3      position;
    gfloat      rotation;
    float3      velocity;
    gboolean    animating;
    guint       action;
    guint       last_action;
    RMesh*      mesh;
    float3      bbox[2];
    WorldNode*  world_node;
};
typedef struct _Entity Entity;

/* --- variables --- */
extern const gchar* HeroNames[];

extern Kernel* kernel;
extern Console* console;
extern Entity* hero;
extern World* manor;

/* --- functions --- */
extern void
hero_spawn();

extern void
hero_kill();

extern void
hero_physic();

extern World*
world_new(
    RMeshGroup*             groups
    );

extern void
world_free(
    World*                  world
    );
    
extern WorldNode*
world_node_get(
    World*                  world,
    float3*                 position
    );
    
extern gboolean
world_node_collide(
    WorldNode*              node_to_test,
    float3*                 bbox,
    float3*                 reaction
    );
    
extern void
world_node_draw(
    float4x4*               view,
    World*                  world,
    WorldNode*              node_to_draw
    );
    
extern void
resources_progress_bar_start(
    guint                   max
    );
    
extern void
resources_progress_bar_stepit();

extern gfloat
resources_progress_bar_get();

extern void
resources_load(
    RResourceManagerValue*  value
    );
    
extern gboolean
engine(
    gpointer        data
    );

extern gboolean
physic(
    gpointer        data
    );
    
extern gboolean
ai(
    gpointer        data
    );
    
extern void
renderer_scene_setup();

extern void
renderer_scene_render();

#endif /* __GLOBALS_H__ */

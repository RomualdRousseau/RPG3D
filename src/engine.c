/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      engine.c
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

/* --- types --- */
typedef struct __ResourceProgressBar _ResourceProgressBar;

/* --- structures --- */
struct __ResourceProgressBar
{
    guint   max;
    guint   current;
};

/* --- variables --- */
static _ResourceProgressBar progress_bar;

/* --- functions --- */
static void
_who(
    gchar** args
    )
{
    r_console_printf("%d: %s\n", kernel->selected_hero, HeroNames[kernel->selected_hero]);
}

static void
_change(
    gchar** args
    )
{
    if(args[1] == NULL)
    {
        kernel->desired_hero = (kernel->selected_hero + 1) % 7;
    }
    else
    {
        kernel->desired_hero = g_ascii_strtoull(args[1], NULL, 10) % 7;
    }
    kernel->state = GAME_CHANGE_HERO;
    r_console_print("OK\n");
}

static void
_list(
    gchar** args
    )
{
    guint i;
    
    for(i = 0; i < 7; i++)
    {
        r_console_printf("%d: %s\n", i, HeroNames[i]);
    }
}

static void
_configure()
{
    r_game_signal_connect(
        "console_list",
        (RGameCallback) _list
        );
    r_game_signal_connect(
        "console_who",
        (RGameCallback) _who
        );
    r_game_signal_connect(
        "console_change",
        (RGameCallback) _change
        );
    
    kernel->actions[ACTION_QUIT] = r_game_action_register(XK_Escape);
    kernel->actions[ACTION_FULLSCREEN_TOGGLE] = r_game_action_register(XK_F1);
    kernel->actions[ACTION_GRAB_INPUT_TOGGLE] = r_game_action_register(XK_F2);
    kernel->actions[ACTION_CONSOLE_MODE_TOGGLE] = r_game_action_register(XK_F3);
    kernel->actions[ACTION_CHANGE_HERO] = r_game_action_register(XK_F4);
    
    kernel->actions[ACTION_HERO_LEFT] = r_game_action_register(XK_Left);
    kernel->actions[ACTION_HERO_RIGHT] = r_game_action_register(XK_Right);
    kernel->actions[ACTION_HERO_JUMP] = r_game_action_register(XK_Q);
    kernel->actions[ACTION_HERO_CROUCH] = r_game_action_register(XK_A);
    kernel->actions[ACTION_HERO_FORWARD] = r_game_action_register(XK_Up);
    kernel->actions[ACTION_HERO_BACKWARD] = r_game_action_register(XK_Down);
}

static void
_load()
{
    resources_progress_bar_start(4);
    console->hud = r_resource_ref("console.hud");
    resources_progress_bar_stepit();
    console->font = r_resource_ref("console.font");
    resources_progress_bar_stepit();
    manor = r_resource_ref("meshes.manor");
    resources_progress_bar_stepit();
    hero_spawn();
    resources_progress_bar_stepit();
}

void
resources_progress_bar_start(
    guint       max
    )
{
    progress_bar.max = max;
    progress_bar.current = 0;
}

void
resources_progress_bar_stepit()
{
    progress_bar.current++;
}

gfloat
resources_progress_bar_get()
{
    return (gfloat) progress_bar.current / (gfloat) progress_bar.max;
}

gboolean
engine(
    gpointer        data
    )
{
    switch(kernel->state)
    {
        case GAME_INIT:
            _configure();
            kernel->state = GAME_LOADING;
            break;

        case GAME_LOADING:
            _load();
            kernel->state = GAME_SCENE;
            break;

        case GAME_SCENE:
            if(*kernel->actions[ACTION_QUIT])
            {
                kernel->state = GAME_DESTROY;
            }
            if(*kernel->actions[ACTION_FULLSCREEN_TOGGLE])
            {
                kernel->fullscreen = !kernel->fullscreen;
                r_game_window_set_fullscreen(kernel->fullscreen);
                *kernel->actions[ACTION_FULLSCREEN_TOGGLE] = R_ACTION_NONE;
            }
            if(*kernel->actions[ACTION_GRAB_INPUT_TOGGLE])
            {
                kernel->grab_input = !kernel->grab_input;
                r_game_window_set_grab_input(kernel->grab_input);
                *kernel->actions[ACTION_GRAB_INPUT_TOGGLE] = R_ACTION_NONE;
            }
            if(*kernel->actions[ACTION_CONSOLE_MODE_TOGGLE])
            {
                console->mode = !console->mode;
                r_game_window_set_console_input(console->mode);
                *kernel->actions[ACTION_CONSOLE_MODE_TOGGLE] = R_ACTION_NONE;
            }
            if(*kernel->actions[ACTION_CHANGE_HERO])
            {
                kernel->desired_hero = (kernel->selected_hero + 1) % 7;
                kernel->state = GAME_CHANGE_HERO;
                *kernel->actions[ACTION_CHANGE_HERO] = R_ACTION_NONE;
            }
            break;
            
        case GAME_CHANGE_HERO:
            hero_kill();
            kernel->selected_hero = kernel->desired_hero;
            hero_spawn();
            kernel->state = GAME_SCENE;
            break;
            
        case GAME_DESTROY:
            r_game_main_quit();
            break;
    }
    return TRUE;
}

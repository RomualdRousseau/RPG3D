/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      main.c
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
#include <errno.h>

/* --- variables --- */
static Kernel _kernel = {GAME_INIT, 0, FALSE, FALSE, 0};
Kernel* kernel = &_kernel;

static Console _console = {NULL, NULL, FALSE};
Console* console = &_console;

/* --- functions --- */
static void
_quit()
{
    kernel->state = GAME_DESTROY;
}

static gboolean
_nice(
    gpointer        data
    )
{
    g_usleep(1000);
    return TRUE;
}

static gboolean
_benchmark(
    gpointer        data
    )
{
    static guint64 last_time = 0;
    guint64 current_time;
    gfloat elasped_time;
    gfloat fps;

    current_time = r_game_current_time();
    if(last_time > 0)
    {
        elasped_time = (gfloat) (current_time - last_time) / (gfloat) G_USEC_PER_SEC;
        fps = (gfloat) kernel->frame_count / elasped_time;
        g_message(
            "%d frames in 5.0 seconds = %.3f FPS",
            (gint) (5.0f * fps),
            fps
            );
    }
    kernel->frame_count = 0;
    last_time = current_time;
    return TRUE;
}

static void
_game_init()
{
    r_console_print(PACKAGE_STRING " linux x-86\n");
    r_console_print(R_CONSOLE_PROMPT);

    r_game_window_set_title(TITLE);
    r_game_window_set_resizeable(TRUE);
    r_game_window_set_grab_input(FALSE);
    r_game_window_resize(WIDTH, HEIGHT);

    r_game_signal_connect(
        "game_quit",
        (RGameCallback)_quit
        );
    r_game_signal_connect(
        "console_quit",
        (RGameCallback) _quit
        );
    r_game_signal_connect(
        "resource_manager_load",
        (RGameCallback) resources_load
        );
    r_game_signal_connect(
        "renderer_scene_setup",
        (RGameCallback) renderer_scene_setup
        );
    r_game_signal_connect(
        "renderer_scene_render",
        (RGameCallback) renderer_scene_render
        );

    g_timeout_add(20, engine, NULL);
    g_timeout_add(20, ai, NULL);
    g_timeout_add(10, physic, NULL);

    g_idle_add(_nice, NULL);

    _benchmark(NULL);
    g_timeout_add_seconds(5, _benchmark, NULL);
}

/**
 * main:
 * @argc:
 * @argv:
 *
 * Entry point of the program.
 *
 * Return value: 0
 **/
int
main(
    gint        argc,
    gchar**     argv
    )
{
    g_message(PACKAGE_STRING " linux x-86");
    r_game_init(argc, argv);
    _game_init();
    r_game_main();
    g_message("Shutdown.");
    return 0;
}

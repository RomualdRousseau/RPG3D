/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      game.c
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

/* --- structures --- */
struct __RGame
{
/* public */
    Display*            display;
    XVisualInfo*        visual;
    guint               frame_time;
/* private */
    gboolean            mainloop_suspended;
    GMainLoop*          mainloop;
    GMutex              lock_signal_vt;
    GHashTable*         signal_vt;
    gboolean            input_console_mode;
};
typedef struct __RGame _RGame;

/* --- variables --- */
static _RGame           self = {NULL, NULL, 0, TRUE, NULL, {0}, NULL, FALSE};
const RGame             game = (RGame)&self;
static gshort           keyboard_keymap[256];
const gint              visual_attributes[][16] =
{
    /*
     * visuals are tried in this order
     */
    /* DoubleBuffer, Color: 24 bits, Depth: 24 bits */
    {GLX_DOUBLEBUFFER, GLX_RGBA, GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_ALPHA_SIZE, 8, GLX_DEPTH_SIZE, 24, None},
    /* DoubleBuffer, Color: 24 bits, Depth: 16 bits */
    {GLX_DOUBLEBUFFER, GLX_RGBA, GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_ALPHA_SIZE, 8, GLX_DEPTH_SIZE, 16, None},
    /* DoubleBuffer, Color: 16 bits, Depth: 16 bits */
    {GLX_DOUBLEBUFFER, GLX_RGBA, GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4, GLX_BLUE_SIZE, 4, GLX_ALPHA_SIZE, 4, GLX_DEPTH_SIZE, 16, None},
    /* SingleBuffer, Color: 16 bits, Depth: 16 bits */
    {                  GLX_RGBA, GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4, GLX_BLUE_SIZE, 4, GLX_ALPHA_SIZE, 4, GLX_DEPTH_SIZE, 16, None},
    {None}
};

/* --- functions --- */
/*
 * _game_thread_init:
 *
 */
static void
_game_thread_init()
{
    if(!g_thread_supported())
    {
        //g_thread_init(NULL);
    }
    if(!XInitThreads())
    {
        g_error("Could not initialize Xlib thread support");
    }
}

/*
 * _game_display_init:
 *
 */
static void
_game_display_init()
{
    gint* best_visual_attributes;
    gint i;

    self.display = XOpenDisplay(NULL);
    if(self.display == NULL)
    {
        g_error("Could not connect to the X server");
    }

    self.visual = NULL;
    for(i = 0; self.visual == NULL; i++)
    {
        best_visual_attributes = (gint*) visual_attributes[i];
        if(*best_visual_attributes == None)
        {
            break;
        }
        self.visual = glXChooseVisual(
            self.display,
            DefaultScreen(self.display),
            best_visual_attributes
            );
    }
    if(self.visual == NULL)
    {
        g_error("Could not find a suitable visual");
    }
}

/*
 * _game_process_event:
 *
 */
static void
_game_process_event(
    gint        msg,
    XEvent*     xevent
    )
{
#ifdef DEBUG
    if(xevent->xany.window != window->window)
    {
        g_debug("DOH!??? What do you mean, Mr. X?");
    }
#endif
    switch(msg)
    {
    case MapNotify:
        self.mainloop_suspended = FALSE;
        r_renderer_resume();
        break;

    case UnmapNotify:
        self.mainloop_suspended = TRUE;
        r_renderer_pause();
        break;

    case ConfigureNotify:
        if((xevent->xconfigure.width != window->width) ||
            (xevent->xconfigure.height != window->height))
        {
            window->width = xevent->xconfigure.width;
            window->height = xevent->xconfigure.height;
            r_renderer_resize(window->width, window->height);
        }
        break;

    case ClientMessage:
        if(xevent->xclient.message_type == XInternAtom(self.display, "WM_PROTOCOLS", False))
        {
            if(xevent->xclient.data.l[0] == XInternAtom(self.display, "WM_DELETE_WINDOW", False))
            {
                r_game_signal_emit("game_quit");
            }
            else if(xevent->xclient.data.l[0] == XInternAtom(self.display, "WM_TAKE_FOCUS", False))
            {
                XSetInputFocus(self.display, window->window, RevertToParent, xevent->xclient.data.l[1]);
            }
        }
        break;

    case KeyPress:
        keyboard_keymap[xevent->xkey.keycode] = R_ACTION_HIT;
        break;

    case KeyRelease:
        keyboard_keymap[xevent->xkey.keycode] = R_ACTION_NONE;
        if(self.input_console_mode)
        {
            r_game_signal_emit2("game_key", &xevent->xkey);
        }
        break;
    }
}

/*
 * _game_idle:
 *
 */
static gboolean
_game_idle(
    gpointer    data
    )
{
    XEvent xevent;

    if(self.mainloop_suspended || (XPending(self.display) > 0))
    {
        do
        {
            XNextEvent(self.display, &xevent);
            _game_process_event(xevent.type, &xevent);
        }
        while(self.mainloop_suspended || (XPending(self.display) > 0));
    }
    else
    {
        r_renderer_update();
    }
    return TRUE;
}

/**
 * r_game_init:
 *
 **/
void
r_game_init(
    gint        argc,
    gchar**     argv
    )
{
    _game_thread_init();
    _game_display_init();

    self.mainloop = g_main_loop_new(NULL, FALSE);
    g_mutex_init(&self.lock_signal_vt);
    self.signal_vt = g_hash_table_new(g_str_hash, g_str_equal);

    r_desktop_init();
    r_window_init(argc, argv);
    r_renderer_init();
    r_modules_init();
    r_resource_manager_init();
    r_console_init();

    g_idle_add(_game_idle, NULL);
}

/**
 * r_game_destroy:
 *
 **/
void
r_game_destroy()
{
    r_console_destroy();
    r_resource_manager_destroy();
    r_modules_destroy();
    r_renderer_destroy();
    r_window_destroy();
    r_desktop_destroy();

    g_hash_table_destroy(self.signal_vt);
    g_mutex_clear(&self.lock_signal_vt);
    g_main_loop_unref(self.mainloop);

    XFree(self.visual);
    XCloseDisplay(self.display);
}

/**
 * r_game_main:
 *
 **/
void
r_game_main()
{
    g_main_loop_run(self.mainloop);
    r_game_destroy();
}

/**
 * r_game_main_quit:
 *
 **/
void
r_game_main_quit()
{
    g_main_loop_quit(self.mainloop);
}

/**
 * r_game_usleep:
 *
 **/
void
r_game_usleep(
    guint                   delay
    )
{
    _game_idle(NULL);
    g_usleep(delay);
}

/**
 * r_game_current_time:
 *
 **/
guint64
r_game_current_time()
{
    GTimeVal now;

    g_get_current_time(&now);
    return now.tv_sec * G_USEC_PER_SEC + now.tv_usec;
}

/**
 * r_game_action_register:
 * @keysym:
 *
 **/
gshort*
r_game_action_register(
    KeySym      keysym
    )
{
    return &keyboard_keymap[XKeysymToKeycode(self.display, keysym)];
}

/**
 * r_game_signal_connect:
 * @signal_name:
 * @handler:
 *
 **/
void
r_game_signal_connect(
    const gchar*            signal_name,
    RGameCallback           callback
    )
{
    g_assert(signal_name != NULL);
    g_assert(callback != NULL);

    g_mutex_lock(&self.lock_signal_vt);
    g_hash_table_insert(self.signal_vt, (gpointer) signal_name, (gpointer) callback);
    g_mutex_unlock(&self.lock_signal_vt);
}

/**
 * r_game_signal_disconnect:
 * @signal_name:
 *
 **/
void
r_game_signal_disconnect(
    const gchar*            signal_name
    )
{
    g_assert(signal_name != NULL);

    g_mutex_lock(&self.lock_signal_vt);
    g_hash_table_remove(self.signal_vt, (gpointer) signal_name);
    g_mutex_unlock(&self.lock_signal_vt);
}

/**
 * r_game_signal_get_address:
 * @signal_name:
 *
 **/
RGameCallback
r_game_signal_get_address(
    const gchar*            signal_name
    )
{
    RGameCallback callback;

    g_assert(signal_name != NULL);

    g_mutex_lock(&self.lock_signal_vt);
    callback = (RGameCallback) g_hash_table_lookup(self.signal_vt, signal_name);
    g_mutex_unlock(&self.lock_signal_vt);
    return callback;
}

/**
 * r_game_signal_get_address_with_default:
 * @signal_name:
 * @default_callback:
 *
 **/
RGameCallback
r_game_signal_get_address_with_default(
    const gchar*            signal_name,
    RGameCallback           default_callback
    )
{
    RGameCallback callback;

    g_assert(signal_name != NULL);
    g_assert(default_callback != NULL);

    callback = r_game_signal_get_address(signal_name);
    return (callback != NULL) ? callback : default_callback;
}

/**
 * r_game_signal_emit:
 * @signal_name:
 *
 **/
void
r_game_signal_emit(
    const gchar*            signal_name
    )
{
    RGameCallback callback;

    g_assert(signal_name != NULL);

    callback = r_game_signal_get_address(signal_name);
    if(callback != NULL)
    {
        callback();
    }
}

/**
 * r_game_signal_emit2_with_default:
 * @signal_name:
 * @user_data:
 *
 **/
void
r_game_signal_emit_with_default(
    const gchar*            signal_name,
    RGameCallback           default_callback
    )
{
    RGameCallback callback;

    g_assert(signal_name != NULL);

    callback = r_game_signal_get_address(signal_name);
    if(callback != NULL)
    {
        callback();
    }
    else
    {
        default_callback();
    }
}

/**
 * r_game_signal_emit2:
 * @signal_name:
 * @user_data:
 *
 **/
void
r_game_signal_emit2(
    const gchar*            signal_name,
    gpointer                user_data
    )
{
    RGameCallback2 callback;

    g_assert(signal_name != NULL);

    callback = (RGameCallback2) r_game_signal_get_address(signal_name);
    if(callback != NULL)
    {
        callback(user_data);
    }
}

/**
 * r_game_signal_emit2_with_default:
 * @signal_name:
 * @user_data:
 *
 **/
void
r_game_signal_emit2_with_default(
    const gchar*            signal_name,
    gpointer                user_data,
    RGameCallback2          default_callback
    )
{
    RGameCallback2 callback;

    g_assert(signal_name != NULL);

    callback = (RGameCallback2) r_game_signal_get_address(signal_name);
    if(callback != NULL)
    {
        callback(user_data);
    }
    else
    {
        default_callback(user_data);
    }
}

/**
 * r_game_window_set_title:
 * @title:
 *
 **/
void
r_game_window_set_title(
    const gchar*            title
    )
{
    g_assert(title != NULL);

    r_window_set_title(title);
}

/**
 * r_game_window_set_fullscreen:
 * @enable:
 *
 **/
void
r_game_window_set_fullscreen(
    gboolean                enable
    )
{
    r_window_set_fullscreen(enable);
}

/**
 * r_game_window_set_grab_input:
 * @enable:
 *
 **/
void
r_game_window_set_grab_input(
    gboolean                enable
    )
{
    r_window_set_grab_input(enable);
}

/**
 * r_game_window_set_console_input:
 * @enable:
 *
 **/
void
r_game_window_set_console_input(
    gboolean                enable
    )
{
    self.input_console_mode = enable;
}

/**
 * r_game_window_set_resizeable:
 * @enable:
 *
 **/
void
r_game_window_set_resizeable(
    gboolean                enable
    )
{
    r_window_set_resizeable(enable);
}

/**
 * r_game_window_resize:
 * @width:
 * @height:
 *
 **/
void
r_game_window_resize(
    guint                   width,
    guint                   height
    )
{
    r_window_resize(width, height);
}



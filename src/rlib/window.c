/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      window.c
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
#include <X11/extensions/Xxf86dga.h>
#include <X11/extensions/xf86vmode.h>

/* --- structures --- */
struct __RWindow
{
/* public */
    Window                  window;
    guint                   width;
    guint                   height;
    gboolean                resizeable;
    gboolean                fullscreen;
    gboolean                grab_input;
/* private */
    gboolean                mouse_dga_supported;
    gboolean                mouse_dga_state;
    gint                    mouse_accel_numerator;
    gint                    mouse_accel_denominator;
    gint                    mouse_threshold;
};

/* --- variables --- */
static struct __RWindow     self = {None, 0, 0, TRUE, FALSE, FALSE, FALSE, FALSE, 0, 0, 0};
const RWindow               window = (RWindow) &self;

/* --- functions --- */
/*
 * _window_null_cursor:
 * @display:
 * @root:
 *
 * Create a transparent cursor
 */
static Cursor
_window_null_cursor(
    Display*    display,
    Window      root
    )
{
    static Cursor null_cursor = None;
    Pixmap cursormask;
    XColor dummycolour;
    XGCValues xgc;
    GC gc;

    if(null_cursor == None)
    {
        cursormask = XCreatePixmap(display, root, 1, 1, 1);
        xgc.function = GXclear;
        gc =  XCreateGC(display, cursormask, GCFunction, &xgc);
        XFillRectangle(display, cursormask, gc, 0, 0, 1, 1);
        dummycolour.pixel = 0;
        dummycolour.red = 0;
        dummycolour.flags = 04;
        null_cursor = XCreatePixmapCursor(
            display,
            cursormask,
            cursormask,
            &dummycolour,
            &dummycolour,
            0,
            0
            );
        XFreePixmap(display,cursormask);
        XFreeGC(display,gc);
    }
    return null_cursor;
}

/*
 * _window_print_info:
 *
 * Prints some info
 */
static void
_window_print_info()
{
    if(self.mouse_dga_supported)
    {
        g_message("Window: Congrats, you have Direct Graphics Access!");
    }
    else
    {
        g_message("Window: Sorry, no Direct Graphics Access possible!");
    }
}

/**
 * r_window_init:
 *
 **/
void
r_window_init(
    gint        argc,
    gchar**     argv
    )
{
    XSetWindowAttributes swa;
    XClassHint class_hint;
    Atom wm_protocols[2];
    gint version_major;
    gint version_minor;

    if (XF86DGAQueryVersion(game->display, &version_major, &version_minor))
    {
        self.mouse_dga_supported = TRUE;
    }

    swa.border_pixel = 0;
    swa.colormap = XCreateColormap(
        game->display,
        DefaultRootWindow(game->display),
        game->visual->visual,
        AllocNone
        );
    swa.event_mask = StructureNotifyMask | FocusChangeMask | KeyPressMask | KeyReleaseMask;
    self.window = XCreateWindow(
        game->display,
        DefaultRootWindow(game->display),
        0, 0, 100, 100, 0,
        game->visual->depth,
        InputOutput,
        game->visual->visual,
        CWBorderPixel | CWColormap | CWEventMask,
        &swa
        );
    if(self.window == None)
    {
        g_error("Could not create a window");
    }

    XSetCommand(game->display, self.window, argv, argc);

    wm_protocols[0] = XInternAtom(game->display, "WM_DELETE_WINDOW", False);
    wm_protocols[1] = XInternAtom(game->display, "WM_TAKE_FOCUS", False);
    XSetWMProtocols(game->display, self.window, wm_protocols, 2);

    class_hint.res_name = "rlib.game.window";
    class_hint.res_class = "rlib.game.window";
    XSetClassHint(game->display, self.window, &class_hint);

    _window_print_info();

    XMapRaised(game->display, self.window);
}

/**
 * r_window_destroy:
 *
 **/
void
r_window_destroy()
{
    XDestroyWindow(game->display, self.window);
}

/**
 * r_window_show:
 *
 **/
void
r_window_show()
{
    XMapRaised(game->display, self.window);
}

/**
 * r_window_hide:
 *
 **/
void
r_window_hide()
{
    XUnmapWindow(game->display, self.window);
}

/**
 * r_window_set_title:
 *
 **/
void
r_window_set_title(
    const gchar*            title
    )
{
    XTextProperty wm_name;

    XStringListToTextProperty((gchar**) &title, 1, &wm_name);
    XSetWMName(game->display, self.window, &wm_name);
    XSetWMIconName(game->display, self.window, &wm_name);
    XFree(wm_name.value);
}

/**
 * r_window_set_grab_input:
 * @enable:
 *
 **/
void
r_window_set_grab_input(
    gboolean            enable
    )
{
    if(self.grab_input == enable)
    {
        return;
    }

    if(enable)
    {
#ifdef DEBUG
    g_debug("Grab input: DGA: %d", self.mouse_dga_supported);
#endif
        XDefineCursor(
            game->display,
            self.window,
            _window_null_cursor(game->display, self.window)
            );

        XGetPointerControl(
            game->display,
            &self.mouse_accel_numerator,
            &self.mouse_accel_denominator,
            &self.mouse_threshold
            );
        XChangePointerControl(game->display, True, True, 1, 1, 0);

        XWarpPointer(
            game->display,
            None,
            self.window,
            0, 0, 0, 0,
            desktop->current_width / 2,
            desktop->current_height / 2
            );

        if(!self.fullscreen)
        {
            XGrabPointer(
                game->display,
                self.window,
                False,
                0,
                GrabModeAsync, GrabModeAsync,
                self.window,
                None,
                CurrentTime
                );

            XGrabKeyboard(
                game->display,
                self.window,
                False,
                GrabModeAsync,
                GrabModeAsync,
                CurrentTime
                );
        }

        if (self.mouse_dga_supported)
        {
            XF86DGADirectVideo(
                game->display,
                DefaultScreen(game->display),
                XF86DGADirectMouse
                );
            XWarpPointer(game->display, None, self.window, 0, 0, 0, 0, 0, 0);
            self.mouse_dga_state = TRUE;
        }
        else
        {
            self.mouse_dga_state = FALSE;
        }

        self.grab_input = TRUE;
    }
    else
    {
#ifdef DEBUG
    g_debug("Ungrab input: DGA: %d", self.mouse_dga_supported);
#endif
        if (self.mouse_dga_supported && self.mouse_dga_state)
        {
            XF86DGADirectVideo(game->display, DefaultScreen(game->display), 0);
        }

        if(!self.fullscreen)
        {
            XUngrabKeyboard(game->display, CurrentTime);
            XUngrabPointer(game->display, CurrentTime);
        }

        XWarpPointer(
            game->display,
            None,
            self.window,
            0, 0, 0, 0,
            desktop->current_width / 2,
            desktop->current_height / 2
            );

        XChangePointerControl(
            game->display,
            True,
            True,
            self.mouse_accel_numerator,
            self.mouse_accel_denominator,
            self.mouse_threshold
            );

        XUndefineCursor(game->display, self.window);

        self.grab_input = FALSE;
    }

    XSync(game->display, False);
}

/**
 * r_window_set_fullscreen:
 * @enable:
 *
 **/
void
r_window_set_fullscreen(
    gboolean            enable
    )
{
    XSetWindowAttributes swa;
    XSizeHints* sizehints;
    XEvent xev;

    if(self.fullscreen == enable)
    {
        return;
    }

    r_renderer_pause();
    
    if(enable)
    {
#ifdef DEBUG
    g_debug("Switch to fullscreen mode");
#endif
        xev.type = ClientMessage;
        xev.xclient.window = self.window;
        xev.xclient.message_type = XInternAtom(game->display, "_NET_WM_STATE", False);
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = 2;
        xev.xclient.data.l[1] = XInternAtom(game->display, "_NET_WM_STATE_FULLSCREEN", False);
        xev.xclient.data.l[2] = 0;	/* no second property to toggle */
        xev.xclient.data.l[3] = 1;	/* source indication: application */
        xev.xclient.data.l[4] = 0;	/* unused */
        if(!XSendEvent(game->display, DefaultRootWindow(game->display), 0, SubstructureRedirectMask | SubstructureNotifyMask, &xev))
        {
            swa.override_redirect = True;
            XChangeWindowAttributes(
                game->display,
                self.window,
                CWOverrideRedirect,
                &swa
                );
            XReparentWindow(
                game->display,
                self.window,
                DefaultRootWindow(game->display),
                0, 0
                );
            XMoveResizeWindow(
                game->display,
                self.window,
                0, 0,
                desktop->current_width,
                desktop->current_height
                );
        }
        
        XWarpPointer(
            game->display,
            None,
            self.window,
            0, 0, 0, 0,
            desktop->current_width / 2,
            desktop->current_height / 2
            );
        XGrabPointer(
            game->display,
            self.window,
            False,
            0,
            GrabModeAsync, GrabModeAsync,
            self.window,
            None,
            CurrentTime
            );
        XGrabKeyboard(
            game->display,
            self.window,
            False,
            GrabModeAsync,
            GrabModeAsync,
            CurrentTime
            );
        XSync(game->display, False);
        
        r_desktop_set_resolution(window->width, window->height, 0);

        self.fullscreen = TRUE;
    }
    else
    {
#ifdef DEBUG
    g_debug("Switch to window mode: %dx%d", self.width, self.height);
#endif
        r_desktop_set_resolution(desktop->default_width, desktop->default_height, 0);
        
        xev.type = ClientMessage;
        xev.xclient.window = self.window;
        xev.xclient.message_type = XInternAtom(game->display, "_NET_WM_STATE", False);
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = 2;
        xev.xclient.data.l[1] = XInternAtom(game->display, "_NET_WM_STATE_FULLSCREEN", False);
        xev.xclient.data.l[2] = 0;	/* no second property to toggle */
        xev.xclient.data.l[3] = 1;	/* source indication: application */
        xev.xclient.data.l[4] = 0;	/* unused */
        if(!XSendEvent(game->display, DefaultRootWindow(game->display), 0, SubstructureRedirectMask | SubstructureNotifyMask, &xev))
        {
            swa.override_redirect = False;
            XChangeWindowAttributes(
                game->display,
                self.window,
                CWOverrideRedirect,
                &swa
                );
            XReparentWindow(
                game->display,
                self.window,
                DefaultRootWindow(game->display),
                0, 0
                );
            if(!self.resizeable)
            {
                sizehints = XAllocSizeHints();
                g_assert(sizehints != NULL);
                sizehints->flags = PMinSize | PMaxSize;
                sizehints->min_width = self.width;
                sizehints->min_height = self.height;
                sizehints->max_width = self.width;
                sizehints->max_height = self.height;
                XSetWMNormalHints(game->display, self.window, sizehints);
                XFree(sizehints);
            }
            XMoveResizeWindow(
                game->display,
                self.window,
                0, 0,
                self.width,
                self.height
                );
        }

        XUngrabKeyboard(game->display, CurrentTime);
        XUngrabPointer(game->display, CurrentTime);
        XWarpPointer(
            game->display,
            None,
            self.window,
            0, 0, 0, 0,
            desktop->current_width / 2,
            desktop->current_height / 2
            );
        XSync(game->display, False);
        
        self.fullscreen = FALSE;
    }

    r_renderer_resume();
}

/**
 * r_window_set_resizeable:
 * @enable:
 *
 **/
void
r_window_set_resizeable(
    gboolean                enable
    )
{
    XSizeHints* sizehints;

    if(self.resizeable == enable)
    {
        return;
    }

    if(!enable)
    {
        sizehints = XAllocSizeHints();
        g_assert(sizehints != NULL);
        sizehints->flags = PMinSize | PMaxSize;
        sizehints->min_width = self.width;
        sizehints->min_height = self.height;
        sizehints->max_width = self.width;
        sizehints->max_height = self.height;
        XSetWMNormalHints(game->display, self.window, sizehints);
        XFree(sizehints);
    }
    else
    {
        XDeleteProperty(
            game->display,
            self.window,
            XInternAtom(game->display, "WM_NORMAL_HINTS", False)
            );
    }
    self.resizeable = enable;
}

/**
 * r_window_resize:
 * @width:
 * @height:
 *
 **/
void
r_window_resize(
    guint                   width,
    guint                   height
    )
{
    XSizeHints* sizehints;

    if(self.fullscreen)
    {
        r_desktop_set_resolution(width, height, 0);
    }
    else
    {
        if((self.width == width) && (self.height == height))
        {
            return;
        }

        if(!self.resizeable)
        {
            sizehints = XAllocSizeHints();
            g_assert(sizehints != NULL);
            sizehints->flags = PMinSize | PMaxSize;
            sizehints->min_width = width;
            sizehints->min_height = height;
            sizehints->max_width = width;
            sizehints->max_height = height;
            XSetWMNormalHints(game->display, self.window, sizehints);
            XFree(sizehints);
        }
        XResizeWindow(game->display, self.window, width, height);
        XFlush(game->display);
    }
}

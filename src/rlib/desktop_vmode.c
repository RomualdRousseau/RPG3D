/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      desktop_vmode.c
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
#include <memory.h>
#include <X11/extensions/Xrandr.h>

/* --- types --- */
typedef struct __RDesktop _RDesktop;

/* --- structures --- */
struct __RDesktop
{
/* public */
    guint                   default_width;
    guint                   default_height;
    guint                   default_rate;
    guint                   current_width;
    guint                   current_height;
    guint                   current_rate;
/* private */
    Rotation                rotation;
    SizeID                  size;
    gshort                  rate;
};

/* --- variables --- */
static _RDesktop    self = {0, 0, 0, 0, 0, 0, 0, 0, 0};
const RDesktop      desktop = (RDesktop) &self;

/* --- functions --- */
/*
 * _desktop_print_info:
 *
 * Prints some info
 */
static void
_desktop_print_info()
{
    g_message("Desktop: VIDEO_MODE    = %dx%d %dHz", self.default_width, self.default_height, self.default_rate);
}

/*
 * r_desktop_init:
 *
 */
void
r_desktop_init()
{
    gint event_base;
    gint error_base;
    gint version_major;
    gint version_minor;
    gint sizes_count;
    XRRScreenSize* sizes;
    XRRScreenConfiguration *sc;
    
    if(!XRRQueryExtension(
        game->display,
        &event_base,
        &error_base
        )
    || !XRRQueryVersion(
        game->display,
        &version_major,
        &version_minor
        ))
    {
        g_error("XRandr not supported");
    }
    
    sc = XRRGetScreenInfo(game->display, DefaultRootWindow(game->display));
    self.size = XRRConfigCurrentConfiguration(sc, &self.rotation);
    self.rate = XRRConfigCurrentRate(sc);
    XRRFreeScreenConfigInfo(sc);

    sizes = XRRSizes(game->display, DefaultScreen(game->display), &sizes_count);
    self.default_width = sizes[self.size].width;
    self.default_height = sizes[self.size].height;
    self.default_rate = self.rate;
    self.current_width = self.default_width;
    self.current_height = self.default_height;
    self.current_rate = self.default_rate;

    _desktop_print_info();
}

/*
 * r_desktop_destroy:
 *
 */
void
r_desktop_destroy()
{
    XRRScreenConfiguration *sc;
    
    if(self.current_width != self.default_width || self.current_height != self.default_height)
    {
        sc = XRRGetScreenInfo(game->display, DefaultRootWindow(game->display));
        XRRSetScreenConfigAndRate(
            game->display,
            sc,
            DefaultRootWindow(game->display),
            self.size,
            self.rotation,
            self.rate,
            CurrentTime
            );
        XRRFreeScreenConfigInfo(sc);
        XSync(game->display, True);
    }
}

/*
 * r_desktop_set_resolution:
 * @width:
 * @height:
 * @rate:
 *
 */
void
r_desktop_set_resolution(
    guint       width,
    guint       height,
    guint       rate
    )
{
    gint sizes_count;
    XRRScreenSize* sizes;
    gint rates_count;
    gshort* rates;
    gint i, d1, d2;
    gint best_size = -1;
    gshort best_rate = -1;
    XRRScreenConfiguration *sc;

    if(width == self.current_width && height == self.current_height)
    {
        return;
    }

#ifdef DEBUG
    g_debug("Old Video Mode: %dx%d %dHz", self.current_width, self.current_height, self.current_rate);
#endif

    if(width == self.default_width && height == self.default_height)
    {
        best_size = self.size;
        best_rate = self.rate;
        self.current_width = self.default_width;
        self.current_height = self.default_height;
        self.current_rate = self.rate;
    }
    else
    {
        sizes = XRRSizes(game->display, DefaultScreen(game->display), &sizes_count);
        d1  = (width - sizes[0].width) * (width - sizes[0].width)
            + (height - sizes[0].height) * (height - sizes[0].height);
        best_size = 0;
        for(i = 1; i < sizes_count; i++)
        {
            d2  = (width - sizes[i].width) * (width - sizes[i].width)
                + (height - sizes[i].height) * (height - sizes[i].height);
            if(d1 > d2)
            {
                d1 = d2;
                best_size = i;
            }
        }
        
        rates = XRRRates(game->display, DefaultScreen(game->display), best_size, &rates_count);
        best_rate = rates[0];
        for(i = 1; i < rates_count; i++)
        {
            if(rates[i] > best_rate)
            {
                best_rate = rates[i];
            }
        }
    
        self.current_width = sizes[best_size].width;
        self.current_height = sizes[best_size].height;
        self.current_rate = best_rate;
    }
    
#ifdef DEBUG
    g_debug("New Video Mode: %dx%d %dHz", self.current_width, self.current_height, self.current_rate);
#endif

    sc = XRRGetScreenInfo(game->display, DefaultRootWindow(game->display));
    XRRSetScreenConfigAndRate(
        game->display,
        sc,
        DefaultRootWindow(game->display),
        best_size,
        self.rotation,
        best_rate,
        CurrentTime
        );
    XRRFreeScreenConfigInfo(sc);
    XSync(game->display, True);
}

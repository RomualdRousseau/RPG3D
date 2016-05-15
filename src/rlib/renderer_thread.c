/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      renderer_thread.c
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

/* --- enums --- */
enum
{
    R_SLEEPING          = 0,
    R_RUNNING           = 1,
    R_TERMINATED        = 2
};

enum
{
    R_PRIORITY_HIGH     = 0,
    R_PRIORITY_NORMAL   = 1
};

/* --- types --- */
typedef struct __RendererThread _RendererThread;

typedef struct __RendererThreadExecuteContext _RendererThreadExecuteContext;

typedef struct __ResizeParams _ResizeParams;

/* --- structures --- */
struct __RendererThread
{
    GThread*        thread;
    GAsyncQueue*    command_queue;
    GMutex          wait_mutex;
    GCond           wait_cond;
    gboolean        terminated;
    gboolean        paused;
};

struct __RendererThreadExecuteContext
{
    gint            priority;
    gint            status;
    GThreadFunc     function;
    gpointer        user_data;
    gpointer        return_value;
    GSourceFunc     completed_function;
};

struct __ResizeParams
{
    guint           width;
    guint           height;
};

/* --- variables --- */
static _RendererThread self = {NULL, NULL, {0}, {0}, FALSE, TRUE};

/* --- functions --- */
/*
 * _renderer_thread_compare_priority:
 *
 */
static gint
_renderer_thread_compare_priority(
    gconstpointer   a,
    gconstpointer   b,
    gpointer        user_data
    )
{
    _RendererThreadExecuteContext* c1 = (_RendererThreadExecuteContext*)a;
    _RendererThreadExecuteContext* c2 = (_RendererThreadExecuteContext*)b;
    return (c1->priority > c2->priority) ? +1 : ((c1->priority == c2->priority) ? 0 : -1);
}

/*
 * _renderer_thread_worker:
 *
 */
static gpointer
_renderer_thread_worker(
    gpointer        data
    )
{
    _RendererThreadExecuteContext* context;

    r_thread_set_cpu_affinity(R_CPU1);

    XLockDisplay(game->display);
    glXMakeCurrent(game->display, window->window, renderer->context);
    XUnlockDisplay(game->display);

    while(!self.terminated)
    {
        if(self.paused)
        {
            context = (_RendererThreadExecuteContext*)g_async_queue_pop(self.command_queue);
        }
        else
        {
            context = (_RendererThreadExecuteContext*)g_async_queue_try_pop(self.command_queue);
        }
        if(context != NULL)
        {
            if(context->function != NULL)
            {
                context->status = R_RUNNING;
                context->return_value = context->function(context->user_data);
            }
            if(context->completed_function == NULL)
            {
                g_mutex_lock(&self.wait_mutex);
                context->status = R_TERMINATED;
                g_cond_broadcast(&self.wait_cond);
                g_mutex_unlock(&self.wait_mutex);
            }
            else
            {
                context->status = R_TERMINATED;
                g_idle_add(context->completed_function, context->return_value);
                g_slice_free(_RendererThreadExecuteContext, context);
            }
        }
        else if(!self.paused)
        {
            r_renderer_render_scene();
            r_renderer_swap_buffers();
        }
    }

    XLockDisplay(game->display);
    glXMakeCurrent(game->display, None, NULL);
    XUnlockDisplay(game->display);

    return NULL;
}

/*
 * _renderer_thread_push_full:
 *
 */
static gpointer
_renderer_thread_push_full(GThreadFunc function, gpointer user_data, gint priority, GSourceFunc completed_function)
{
    _RendererThreadExecuteContext* context;
    gpointer result = NULL;
    
    if(g_thread_self() == self.thread)
    {
        result = function(user_data);
        if(completed_function != NULL)
        {
            g_idle_add(completed_function, user_data);
        }
        return result;
    }
    else
    {
        context = g_slice_new0(_RendererThreadExecuteContext);
        context->priority = priority;
        context->status = R_SLEEPING;
        context->function = function;
        context->user_data = user_data;
        context->completed_function = completed_function;

        g_async_queue_push_sorted(self.command_queue, context, _renderer_thread_compare_priority, NULL);

        if(completed_function == NULL)
        {
            g_mutex_lock(&self.wait_mutex);
            while(context->status != R_TERMINATED)
            {
                g_cond_wait(&self.wait_cond, &self.wait_mutex);
            }
            g_mutex_unlock(&self.wait_mutex);
            result = context->return_value;
            g_slice_free(_RendererThreadExecuteContext, context);
        }

        return result;
    }
}

/*
 * _renderer_thread_resize_delegate:
 *
 */
static gpointer
_renderer_thread_resize_delegate(
    gpointer        data
    )
{
    _ResizeParams* params = (_ResizeParams*)data;

    r_renderer_resize_viewport(params->width, params->height);
    g_free(params);
    return NULL;
}

/*
 * _renderer_thread_init:
 *
 */
static void
_renderer_thread_init()
{
    r_thread_set_cpu_affinity(R_CPU0);

    glXMakeCurrent(game->display, None, NULL);

    g_mutex_init(&self.wait_mutex);
    g_cond_init(&self.wait_cond);
    self.command_queue = g_async_queue_new();
    self.thread = g_thread_new("rlib_thread", _renderer_thread_worker, NULL);
}

/*
 * _renderer_thread_destroy:
 *
 */
static void
_renderer_thread_destroy()
{
    self.terminated = TRUE;
    g_thread_join(self.thread);
    g_async_queue_unref(self.command_queue);
    g_cond_clear(&self.wait_cond);
    g_mutex_clear(&self.wait_mutex);

    glXMakeCurrent(game->display, window->window, renderer->context);
}

/*
 * _renderer_thread_resize:
 *
 */
static void
_renderer_thread_resize(
    guint               width,
    guint               height
    )
{
    _ResizeParams* params;

    params = g_new0(_ResizeParams, 1);
    params->width = width;
    params->height = height;

    _renderer_thread_push_full(
        _renderer_thread_resize_delegate,
        params,
        R_PRIORITY_HIGH,
        NULL
        );
}

/*
 * _renderer_thread_pause:
 *
 */
static void
_renderer_thread_pause()
{
    self.paused = TRUE;
    _renderer_thread_push_full(
        NULL,
        NULL,
        R_PRIORITY_NORMAL,
        NULL
        );
}

/*
 * _renderer_thread_resume:
 *
 */
static void
_renderer_thread_resume()
{
    self.paused = FALSE;
    _renderer_thread_push_full(
        NULL,
        NULL,
        R_PRIORITY_HIGH,
        NULL
        );
}

/*
 * _renderer_thread_execute:
 *
 */
static gpointer
_renderer_thread_execute(
    GThreadFunc         function,
    gpointer            user_data,
    GSourceFunc         completed_function
    )
{
    g_assert(function != NULL);
    
    return _renderer_thread_push_full(
        function,
        user_data,
        R_PRIORITY_HIGH,
        completed_function
        );
}

/*
 * _create_instance:
 *
 */
static RRenderer
_create_instance()
{
    RRenderer singleton = renderer;
    singleton->init = _renderer_thread_init;
    singleton->destroy = _renderer_thread_destroy;
    singleton->update = NULL;
    singleton->pause = _renderer_thread_pause;
    singleton->resume = _renderer_thread_resume;
    singleton->resize = _renderer_thread_resize;
    singleton->execute = _renderer_thread_execute;
    return singleton;
}

/*
 * renderer_thread_factory:
 *
 */
RRendererFactory renderer_thread_factory =
{
    "RendererThread",
    "multi threaded renderer",
    _create_instance
};

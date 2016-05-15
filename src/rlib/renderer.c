/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      renderer.c
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
struct __RRenderer
{
/* public */
    GLXContext                  context;
    void                        (*init)();
    void                        (*destroy)();
    void                        (*update)();
    void                        (*pause)();
    void                        (*resume)();
    void                        (*resize)(guint width, guint height);
    gpointer                    (*wait)(gpointer handle);
    gpointer                    (*execute)(GThreadFunc func, gpointer data, gpointer* return_handle);
/* private */
    gint                        draw_buffer;
    gboolean                    swap_vsync;
};

/* --- variables --- */
static struct __RRenderer       self = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, GL_BACK, FALSE};
const RRenderer                 renderer = (RRenderer) &self;
static RRendererFactory*        renderer_factories[] =
{
    &renderer_default_factory,
    &renderer_thread_factory,
    NULL
};
static RRendererFactory*        renderer_factory = NULL;

/* --- functions --- */
/*
 * _renderer_render_scene_default:
 *
 * Default rendering function; just display a blank screen
 */
static void
_renderer_render_scene_default()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/*
 * _renderer_print_info:
 *
 */
static void
_renderer_print_info()
{
    g_message("Renderer: GL_RENDERER  = %s", (gchar*) glGetString(GL_RENDERER));

    g_message("Renderer: GL_VENDOR    = %s", (gchar*) glGetString(GL_VENDOR));

    g_message("Renderer: GL_VERSION   = %s", (gchar*) glGetString(GL_VERSION));

    g_message("Renderer: GLEW_VERSION = %s", glewGetString(GLEW_VERSION));

    g_message("Renderer: GLX_VISUAL   = 0x%02X (see glxinfo)", (gint)game->visual->visualid);

    if(sysconf(_SC_NPROCESSORS_ONLN) > 1)
    {
        g_message("Renderer: Congrats, you have Multi-CPU!");
    }
    else
    {
        g_message("Renderer: Sorry, no Multi-CPU possible!");
    }

    if(self.draw_buffer == GL_BACK)
    {
        g_message("Renderer: Congrats, you have Double Buffering!");
    }
    else
    {
        g_message("Renderer: Sorry, no Double Buffering possible!");
    }

    if(glXIsDirect(game->display, self.context))
    {
        g_message("Renderer: Congrats, you have Direct Rendering!");
    }
    else
    {
        g_message("Renderer: Sorry, no Direct Rendering possible!");
    }

    if(GLEW_ARB_vertex_buffer_object)
    {
        g_message("Renderer: Congrats, you have Vertex Buffer Object!");
    }
    else
    {
        g_message("Renderer: Sorry, no Vertex Buffer Object possible!");
    }
#ifdef VSYNC
    if(self.swap_vsync)
    {
        g_message("Renderer: Congrats, you have VSYNC");
    }
    else
    {
        g_message("Renderer: Sorry, no VSYNC possible!");
    }
#endif
    g_message("Renderer: %s: %s", renderer_factory->name, renderer_factory->description);
}

static void
_renderer_create_default_renderer()
{
    if(r_thread_get_cpu_count() > 1)
    {
        renderer_factory = renderer_factories[1];
    }
    else
    {
        renderer_factory = renderer_factories[0];
    }
    /* renderer = */ renderer_factory->create_instance();
}

/**
 * r_renderer_init:
 *
 **/
void
r_renderer_init()
{
    setenv("__GL_SYNC_TO_VBLANK", "0", 1);

    self.context = glXCreateContext(
        game->display,
        game->visual,
        NULL,
        GL_TRUE
        );
    if(self.context == NULL)
    {
        g_error("Could not create GL context");
    }
    glXMakeCurrent(game->display, window->window, self.context);

    if(glewInit() != GLEW_OK)
    {
        g_error("Could not initialize GLEW");
    }

#ifdef VSYNC
    if(GLX_SGI_swap_control)
    {
        self.swap_vsync = TRUE;
        glXSwapIntervalSGI(1);
    }
    else
    {
        self.swap_vsync = FALSE;
    } 
#endif

#ifdef BACK_BUFFER
    glGetIntegerv(GL_DRAW_BUFFER, &self.draw_buffer);
#else
    self.draw_buffer = GL_FRONT;
#endif
    glDrawBuffer(self.draw_buffer);

    _renderer_create_default_renderer();
    _renderer_print_info();

    renderer->init();
}

/**
 * r_renderer_destroy:
 *
 **/
void
r_renderer_destroy()
{
    renderer->destroy();

    if(self.context != NULL)
    {
        glXMakeCurrent(game->display, None, NULL);
        glXDestroyContext(game->display, self.context);
        self.context = NULL;
    }
}

/**
 * r_renderer_resize_viewport:
 *
 **/
void
r_renderer_resize_viewport(
    guint       width,
    guint       height
    )
{
    float4x4 projection;
    gdouble aspect;
    gdouble y;
    gdouble x;

#ifdef DEBUG
    g_debug("Resize Viewport: %dx%d", width, height);
#endif

    if(height == 0)
    {
        aspect = (gdouble)width;
    }
    else
    {
        aspect = (gdouble)width / (gdouble)height;
    }
    y = 0.1f * tan(45.0f * 0.5f * DEG2RAD);
    x = aspect * y;
    
    r_matrix_frustum_set(&projection, -x, x, -y, y, 0.1f, 100.0f);
    r_frustum_update_projection(&projection);
    
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((GLfloat*) &projection);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glFlush();
}

/**
 * r_renderer_render_scene:
 *
 **/
void
r_renderer_render_scene()
{
    static RGameCallback renderer_render_scene_impl = _renderer_render_scene_default;
    RGameCallback callback;
    
    callback = r_game_signal_get_address_with_default(
            "renderer_scene_render",
            _renderer_render_scene_default
            );
    if(renderer_render_scene_impl != callback)
    {
        renderer_render_scene_impl = callback;
        r_game_signal_emit("renderer_scene_setup");
    }
#ifdef DEBUG
    g_debug("--------- Begin Frame ----------");
#endif
    renderer_render_scene_impl();
}

/**
 * r_renderer_swap_buffers:
 *
 **/
void
r_renderer_swap_buffers()
{
    static guint64 __t1 = 0;
    guint64 __t2;

    if(self.draw_buffer == GL_BACK)
    {
        glXSwapBuffers(game->display, window->window);
    }
    else
    {
        glFlush();
    }
#ifdef DEBUG
    glFinish();
    g_debug("--------- End Frame ------------");
#endif

    __t1 = (__t1 == 0) ? r_game_current_time() : __t1;
    __t2 = r_game_current_time();
    game->frame_time = __t2 - __t1;
    __t1 = __t2;
}

/**
 * r_renderer_begin_2D:
 *
 **/
void
r_renderer_begin_2D()
{
    float4x4 projection;
    
    r_matrix_ortho_set(&projection, 0, 1000, 0, 1000, -1.0, 1.0);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf((GLfloat*) &projection);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

/**
 * r_renderer_end_2D:
 *
 **/
void
r_renderer_end_2D()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

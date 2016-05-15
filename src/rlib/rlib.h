/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      rlib.h
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

#ifndef __RLIB_H__
#define __RLIB_H__

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <X11/Xlib.h>
#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <glib.h>

#include <math3d/math3d.h>

/* RUtility */

enum
{
    R_CPU0              = 0,
    R_CPU1              = 1,
    R_CPU2              = 2
};

extern int
r_thread_get_cpu_count();

extern void
r_thread_set_cpu_affinity(
    gint            cpu
    );

/* Modules */

struct _RModule
{
    gpointer        (*load_from_file)(const gchar* file_name);
};
typedef struct _RModule RModule;

struct _RModuleFactory
{
    gchar*          name;
    gchar*          description;
    RModule*        (*get_instance)();
    gboolean        (*accept_file)(const gchar* file_name);
};
typedef struct _RModuleFactory RModuleFactory;

extern void
r_modules_init();

extern void
r_modules_destroy();

extern RModule*
r_modules_lookup(
    const gchar*      file_name
    );
    
/* RDesktop */

struct _RDesktop
{
    guint                   default_width;
    guint                   default_height;
    guint                   default_rate;
    guint                   current_width;
    guint                   current_height;
    guint                   current_rate;
};
typedef struct _RDesktop*   RDesktop;
extern const RDesktop       desktop;

extern void
r_desktop_init();

extern void
r_desktop_destroy();

extern void
r_desktop_set_resolution(
    guint                   width,
    guint                   height,
    guint                   rate
    );

/* RWindow */

struct _RWindow
{
    Window                  window;
    guint                   width;
    guint                   height;
    gboolean                resizeable;
    gboolean                fullscreen;
    gboolean                grab_input;
};
typedef struct _RWindow*    RWindow;
extern const RWindow        window;

extern void
r_window_init();

extern void
r_window_destroy();

extern void
r_window_show();

extern void
r_window_hide();

extern void
r_window_set_title(
    const gchar*            title
    );

extern void
r_window_set_fullscreen(
    gboolean                enable
    );

extern void
r_window_set_grab_input(
    gboolean                enable
    );

extern void
r_window_set_resizeable(
    gboolean                enable
    );

extern void
r_window_resize(
    guint                   width,
    guint                   height
    );
    
/* RGame */

enum
{
    R_ACTION_NONE       = 0,
    R_ACTION_HIT        = 32767
};

typedef void (*RGameCallback)();

typedef void (*RGameCallback2)(gpointer);

struct _RGame
{
    Display*                display;
    XVisualInfo*            visual;
    guint                   frame_time;
};
typedef struct _RGame*      RGame;
extern const RGame          game;

extern void
r_game_init(
    gint                    argc,
    gchar**                 argv
    );

extern void
r_game_destroy();

extern void
r_game_main();

extern void
r_game_main_quit();

extern void
r_game_usleep(
    guint                   delay
    );

extern guint64
r_game_current_time();

extern gshort*
r_game_action_register(
    KeySym                  keysym
    );

extern void
r_game_signal_connect(
    const gchar*            signal_name,
    RGameCallback           handler
    );

extern void
r_game_signal_disconnect(
    const gchar*            signal_name
    );

extern RGameCallback
r_game_signal_get_address(
    const gchar*            signal_name
    );

extern RGameCallback
r_game_signal_get_address_with_default(
    const gchar*            signal_name,
    RGameCallback           default_callback
    );

extern void
r_game_signal_emit(
    const gchar*            signal_name
    );
    
extern void
r_game_signal_emit_with_default(
    const gchar*            signal_name,
    RGameCallback           default_callback
    );

extern void
r_game_signal_emit2(
    const gchar*            signal_name,
    gpointer                user_data
    );
    
extern void
r_game_signal_emit2_with_default(
    const gchar*            signal_name,
    gpointer                user_data,
    RGameCallback2          default_callback
    );

extern void
r_game_window_set_title(
    const gchar*            title
    );

extern void
r_game_window_set_fullscreen(
    gboolean                enable
    );

extern void
r_game_window_set_grab_input(
    gboolean                enable
    );

extern void
r_game_window_set_console_input(
    gboolean                enable
    );

extern void
r_game_window_set_resizeable(
    gboolean                enable
    );

extern void
r_game_window_resize(
    guint                   width,
    guint                   height
    );

/* RRenderer */

struct _RRenderer
{
    GLXContext              context;
    void                    (*init)();
    void                    (*destroy)();
    void                    (*update)();
    void                    (*pause)();
    void                    (*resume)();
    void                    (*resize)(guint width, guint height);
    gpointer                (*execute)(GThreadFunc func, gpointer data, GSourceFunc completed_function);
};
typedef struct _RRenderer*  RRenderer;

struct _RRendererFactory
{
    gchar*                  name;
    gchar*                  description;
    RRenderer               (*create_instance)();
};
typedef struct _RRendererFactory RRendererFactory;

extern RRendererFactory     renderer_default_factory;
extern RRendererFactory     renderer_thread_factory;
extern const RRenderer      renderer;

extern void
r_renderer_init();

extern void
r_renderer_destroy();

extern void
r_renderer_resize_viewport(
    guint               width,
    guint               height
    );

extern void
r_renderer_render_scene();

extern void
r_renderer_swap_buffers();

static inline void
r_renderer_update()
{
    if(renderer->update != NULL) renderer->update();
}

static inline void
r_renderer_pause()
{
    if(renderer->pause != NULL) renderer->pause();
}

static inline void
r_renderer_resume()
{
    if(renderer->resume != NULL) renderer->resume();
}

static inline void
r_renderer_resize(guint width, guint height)
{
    if(renderer->resize != NULL) renderer->resize(width, height);
}

static inline gpointer
r_renderer_execute(GThreadFunc function, gpointer data)
{
    return (renderer->execute != NULL) ? renderer->execute(function, data, NULL) : NULL;
}

static inline gpointer
r_renderer_execute_full(GThreadFunc function, gpointer data, GSourceFunc completed_function)
{
    return (renderer->execute != NULL) ? renderer->execute(function, data, completed_function) : NULL;
}

extern void
r_renderer_begin_2D();

extern void
r_renderer_end_2D();
    
/* RImage */

struct _RImage
{
    guint           width;
    guint           height;
    guint           bytes_per_pixel;
    guint8*         pixel_data;
};
typedef struct _RImage RImage;

extern RImage*
r_image_new(
    guint           width,
    guint           height,
    guint           bytes_per_pixel
    );

extern RImage*
r_image_new_from_file(
    const gchar*    file_name
    );

extern void
r_image_free(
    RImage*         image
    );

/* RTexture */

enum
{
    R_TEXTURE_NONE      = 0
};

extern GLuint
r_texture_new(
    RImage*         image,
    gint            min_filter,
    gint            mag_filter,
    gboolean        wrap,
    gboolean        free_image
    );

extern GLuint
r_texture_new_mipmap(
    RImage*         image,
    gint            min_filter,
    gint            mag_filter,
    gboolean        wrap,
    gboolean        free_image
    );

extern void
r_texture_free(
    GLuint          texture
    );

extern void
r_texture_replace(
    GLuint          texture,
    RImage*         image,
    gboolean        free_image
    );

/* RMaterial */

struct _RMaterial
{
    guint                   ref;
    GLuint                  texture;
    float4                  color;
};
typedef struct _RMaterial   RMaterial;

extern RMaterial*
r_material_new();

extern RMaterial*
r_material_new_from_file(
    const gchar*            file_name
    );

extern void
r_material_free(
    RMaterial*              material
    );

/* RMesh */

struct _RMeshElement
{
    float3                  point;
    float3                  normal;
    float2                  texcoord;
};
typedef struct _RMeshElement RMeshElement;

struct _RMeshPart
{
    RMaterial*              skin;
    guint                   offset;
    guint                   count;
};
typedef struct _RMeshPart RMeshPart;

struct _RMesh
{
    guint                   vertice_count;
    guint                   frames_count;
    guint                   parts_count;
    guint                   triangles_count;
    RMeshElement**          frames;
    RMeshPart*              parts;
    guint*                  triangles;
    gfloat                  anim_time;
    
};
typedef struct _RMesh       RMesh;

extern gboolean
r_mesh_element_equals(
    RMeshElement*           e1,
    RMeshElement*           e2
    );

extern gboolean
r_mesh_element_insert(
    RMeshElement*           elements,
    guint                   index,
    RMeshElement*           element,
    guint*                  result_index
    );
    
extern void
r_mesh_element_replace(
    RMeshElement*           elements,
    guint                   index,
    RMeshElement*           element
    );

extern RMesh*
r_mesh_new(
    guint                   frames_count,
    guint                   vertice_count,
    guint                   parts_count,
    guint                   triangles_count
    );
    
extern RMesh*
r_mesh_new_from_file(
    const gchar*            file_name,
    RMaterial*              default_skin
    );

extern RMesh*
r_mesh_new_from_files(
    const gchar**           file_names,
    RMaterial*              default_skin
    );
    
extern void
r_mesh_free(
    RMesh*                  mesh
    );

extern void
r_mesh_compute_bbox(
    RMesh*                  mesh,
    guint                   frame,
    float3*                 bbox_result
    );

extern gboolean
r_mesh_collide(
    RMesh*                  mesh,
    guint                   frame,
    float3*                 bbox,
    float3*                 reaction
    );

extern void
r_mesh_draw(
    float4x4*               view,
    RMesh*                  mesh
    );
    
extern gboolean
r_mesh_draw_full(
    float4x4*               view,
    RMesh*                  mesh,
    guint                   frame_first,
    guint                   frame_last,
    guint                   frame_fps,
    gboolean                repeat_mode
    );

/* RMeshGroup */

struct _RMeshGroup
{
    GHashTable*             groups;
    
};
typedef struct _RMeshGroup  RMeshGroup;

extern RMeshGroup*
r_meshgroup_new();

extern RMeshGroup*
r_meshgroup_new_from_file(
    const gchar*            file_name,
    RMaterial*              default_skin
    );
    
extern void
r_meshgroup_free(
    RMeshGroup*             meshgroup
    );
    
extern RMesh*
r_meshgroup_get(
    RMeshGroup*             meshgroup,
    const gchar*            group_name
    );

/* RSurface */

struct _RSurface
{
    GLuint                  texture;
    gboolean                alpha;
    gfloat                  x;
    gfloat                  y;
    gfloat                  width;
    gfloat                  height;
};
typedef struct _RSurface    RSurface;

extern RSurface*
r_surface_new();

extern RSurface*
r_surface_new_from_file(
    const gchar*            file_name
    );

extern void
r_surface_free(
    RSurface*               surface
);

extern void
r_surface_draw(
    RSurface*               surface,
    gint                    x,
    gint                    y,
    guint                   width,
    guint                   height
    );

/* RFont */

struct _RFont
{
    GLuint                  texture;
    gboolean                alpha;
    guint                   char_width;
    guint                   char_height;
};
typedef struct _RFont       RFont;

extern RFont*
r_font_new();

extern RFont*
r_font_new_from_file(
    const gchar*            file_name
    );

extern void
r_font_free(
    RFont*                  font
);

extern void
r_font_draw_char(
    RFont*                  font,
    guint                   font_size,
    gint                    x,
    gint                    y,
    const gchar             c
    );

extern void
r_font_draw_string(
    RFont*                  font,
    guint                   font_size,
    gint                    x,
    gint                    y,
    const gchar*            s
    );

/* RConsole */

#define R_CONSOLE_PROMPT    "# "

extern void
r_console_init();

extern void
r_console_destroy();

extern void
r_console_goto(
    guint               x,
    guint               y
    );

extern void
r_console_putchar(
    const gchar         c
    );

extern void
r_console_print(
    const gchar*        s
    );
    
extern void
r_console_printf(
    const gchar*        format,
    ...
    );

extern void
r_console_draw(
    RFont*      font,
    gint        x,
    gint        y,
    guint       width,
    guint       height
    );

/* RResourceManager */

enum
{
    R_RESOURCE_NONE      = 0,
    R_RESOURCE_MATERIAL  = 1,
    R_RESOURCE_MESH      = 2,
    R_RESOURCE_MESHGROUP = 3,
    R_RESOURCE_SURFACE   = 4,
    R_RESOURCE_FONT      = 5,
    R_RESOURCE_CUSTOM    = 10
};

struct _RResourceManagerValue
{
    guint           user_ref;
    guint           type;
    gchar*          name;
    gchar*          link;
    gpointer        data;
    GDestroyNotify  custom_free_func;
};
typedef struct _RResourceManagerValue RResourceManagerValue;

typedef void (*RResourceCallback)(RResourceManagerValue* value);

extern void
r_resource_manager_init();

extern void
r_resource_manager_destroy();

extern void
r_resource_manager_cleanup();

extern gpointer
r_resource_ref(
    const gchar*            key
    );

extern void
r_resource_unref(
    
    const gchar*            key
    );
    
extern void
r_resource_link(
    RResourceManagerValue*  value,
    const gchar*            key
    );

extern void
r_resource_default_unload(
    RResourceManagerValue*  value
    );
    
extern void
r_resource_material_load(
    RResourceManagerValue*  value,
    const gchar*            material_file_name
    );

extern void
r_resource_mesh_load(
    RResourceManagerValue*  value,
    const gchar*            model_file_name,
    RMaterial*              skin
    );

extern void
r_resource_mesh_load_multiple(
    RResourceManagerValue*  value,
    const gchar**           model_file_names,
    RMaterial*              skin
    );
    
extern void
r_resource_meshgroup_load(
    RResourceManagerValue*  value,
    const gchar*            model_file_name,
    RMaterial*              skin
    );
    
extern void
r_resource_surface_load(
    RResourceManagerValue*  value,
    const gchar*            surface_file_name,
    gfloat                  x,
    gfloat                  y,
    gfloat                  width,
    gfloat                  height
    );
        
extern void
r_resource_font_load(
    RResourceManagerValue*  value,
    const gchar*            font_file_name,
    guint                   char_width,
    guint                   char_height
    );

#endif /* __RLIB_H__ */

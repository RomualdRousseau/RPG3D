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

/* --- functions --- */
void
renderer_scene_setup()
{
    gfloat LightAmbient[]= {1.0f, 1.0f, 1.0f, 1.0f};
    gfloat LightDiffuse[]= {1.0f, 1.0f, 0.99f, 1.0f};
    gfloat lightPos[] = {0.0f, 0.0f, 0.0f, 1.0f};
    gfloat fogColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};  
    
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);

    glClearColor(fogColor[0], fogColor[1], fogColor[2], fogColor[3]);
    glClearDepth(1.0f);
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glEnable(GL_LIGHT0);
    
    glHint(GL_FOG_HINT, GL_DONT_CARE);
    glFogi(GL_FOG_MODE, GL_EXP);
    glFogi(GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.15f);
    glEnable(GL_FOG);   
    
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void
renderer_scene_render()
{
    float4x4 matrix;
    float3 p1 = {0.0f, 1.0f, 0.0f};
    float3 p2 = {-hero->position.x, -hero->position.y, -hero->position.z};
    float3 p3 = {0.0f, -0.4f, -2.0f};

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switch(kernel->state)
    {
        case GAME_LOADING:
            r_renderer_begin_2D();
            glBegin(GL_QUADS);
            glVertex2i(50, 220);
            glVertex2i(900 * resources_progress_bar_get() + 50, 220);
            glVertex2i(900 * resources_progress_bar_get() + 50, 250);
            glVertex2i(50, 250);
            glEnd();
            r_renderer_end_2D();
            break;

        case GAME_SCENE:
            if(hero->action != hero->last_action)
            {
                hero->mesh->anim_time = 0.0f;
                hero->last_action = hero->action;
            }
            
            glEnable(GL_LIGHTING);

            if(hero->world_node != NULL)
            {
                r_matrix_identity_set(&matrix);
                r_matrix_translate(&matrix, &p3);
                r_matrix_rotate(&matrix, -hero->rotation, &p1);
                r_matrix_translate(&matrix, &p2);
                world_node_draw(&matrix, manor, hero->world_node);
            }

            r_matrix_identity_set(&matrix);
            r_matrix_translate(&matrix, &p3);
            r_matrix_rotate(&matrix, -90.0f, &p1);
            if(hero->action == ENTITY_ACTION_NONE)
            {
                hero->animating = r_mesh_draw_full(&matrix, hero->mesh, 0, 39, 9, TRUE);
            }
            else if(hero->action == ENTITY_ACTION_RUNNING)
            {
                hero->animating = r_mesh_draw_full(&matrix, hero->mesh, 40, 45, 10, TRUE);
            }
            else if(hero->action == ENTITY_ACTION_JUMPING)
            {
                hero->animating = r_mesh_draw_full(&matrix, hero->mesh, 66, 71, 7, FALSE);
            }
            else if(hero->action == ENTITY_ACTION_FALLING)
            {
                hero->animating = r_mesh_draw_full(&matrix, hero->mesh, 54,  57,  7, FALSE);
            }
            glDisable(GL_LIGHTING);
            
            r_renderer_begin_2D();
            r_surface_draw(console->hud, 0, 0, 1000, 1000);
            r_font_draw_string(console->font, 16, 992-16*4, 992-16, "DEMO");
            if(console->mode)
            {
                glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
                r_console_draw(console->font, 0, 0, 1000, 1000);
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            }
            r_renderer_end_2D();
            break;
    }

    kernel->frame_count++;
}

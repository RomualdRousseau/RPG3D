/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      hero.c
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

const gchar* HeroNames[] =
{
    "meshes.hero1",
    "meshes.hero2",
    "meshes.hero3",
    "meshes.hero4",
    "meshes.hero5",
    "meshes.hero6",
    "meshes.hero7"
};

/* --- variables --- */
static Entity _hero = {{0.0f, 4.0f, 0.0f}, 0.0f, {0.0f, 0.0f, 0.0f}, TRUE, ENTITY_ACTION_NONE, ENTITY_ACTION_NONE, NULL, {{0.0f}, {0.0f}}, NULL};
Entity* hero = &_hero;

/* --- functions --- */
void
hero_spawn()
{
    hero->mesh = r_resource_ref(HeroNames[kernel->selected_hero]);
    r_mesh_compute_bbox(hero->mesh, 0, hero->bbox);
    hero->bbox[1].x *= 2.0f;
    hero->bbox[1].z *= 2.0f;
    hero->world_node = world_node_get(manor, &hero->position);
}

void
hero_kill()
{
    r_resource_unref(HeroNames[kernel->selected_hero]);
}

void
hero_physic()
{
    float3 bbox[2];
    float3 reaction;
    gboolean do_something = FALSE;
    
    hero->velocity.x = 0.0f;
    if(hero->velocity.y > 0.0f)
    {
        hero->velocity.y += -0.0004f;
    } 
    hero->velocity.z = 0.0f;
    
    if(!console->mode)
    {
        if(hero->action != ENTITY_ACTION_FALLING)
        {
            if(*kernel->actions[ACTION_HERO_LEFT])
            {
                if(hero->action != ENTITY_ACTION_JUMPING)
                {
                    hero->action = ENTITY_ACTION_RUNNING;
                }
                hero->rotation += 0.8f;
                do_something = TRUE;
            }
            if(*kernel->actions[ACTION_HERO_RIGHT])
            {
                if(hero->action != ENTITY_ACTION_JUMPING)
                {
                    hero->action = ENTITY_ACTION_RUNNING;
                }
                hero->rotation -= 0.8f;
                do_something = TRUE;
            }
            if(*kernel->actions[ACTION_HERO_FORWARD])
            {
                if(hero->action != ENTITY_ACTION_JUMPING)
                {
                    hero->action = ENTITY_ACTION_RUNNING;
                }
                hero->velocity.x = -0.04f * sin(hero->rotation * DEG2RAD);
                hero->velocity.z = -0.04f * cos(hero->rotation * DEG2RAD);
                do_something = TRUE;
            }
            if(*kernel->actions[ACTION_HERO_BACKWARD])
            {
                if(hero->action != ENTITY_ACTION_JUMPING)
                {
                    hero->action = ENTITY_ACTION_RUNNING;
                }
                hero->velocity.x = +0.04f * sin(hero->rotation * DEG2RAD);
                hero->velocity.z = +0.04f * cos(hero->rotation * DEG2RAD);
                do_something = TRUE;
            }
            if(*kernel->actions[ACTION_HERO_JUMP])
            {
                if(hero->action != ENTITY_ACTION_JUMPING)
                {
                    hero->action = ENTITY_ACTION_JUMPING;
                }
                if((hero->velocity.y == 0.0f))
                {
                    hero->velocity.y = +0.15f;
                }
                do_something = TRUE;
            }
        }
    }
    
    if(!do_something)
    {
        switch(hero->action)
        {
            case ENTITY_ACTION_JUMPING:
                hero->action = ENTITY_ACTION_JUMPING;
                break;
                
            case ENTITY_ACTION_FALLING:
                if(!hero->animating)
                {
                    hero->action = ENTITY_ACTION_NONE;
                }
                break;
                
            default:
                hero->action = ENTITY_ACTION_NONE;
        }
    }
    
    hero->position.x += hero->velocity.x;
    hero->position.y += hero->velocity.y - 0.0981f;
    hero->position.z += hero->velocity.z;
    
    hero->world_node = world_node_get(manor, r_bbox_translate(hero->bbox, &hero->position, bbox));
    if(world_node_collide(hero->world_node, bbox, &reaction))
    {
        hero->position.x += reaction.x;
        hero->position.y += reaction.y;
        hero->position.z += reaction.z;
        if(hero->action == ENTITY_ACTION_JUMPING)
        {
            if(reaction.y < 0.0f)
            {
                hero->velocity.y = -EPSILON;
            }
            if(reaction.y > 0.0f)
            {
                hero->velocity.y = 0.0f;
                hero->action = ENTITY_ACTION_FALLING;
            }
        }
    }
}

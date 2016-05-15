/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * world.c
 *
 * Copyright 2012 Romuald Rousseau <romuald@you-are-a-wizard>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include <globals.h>

/* --- variables --- */
World* manor;

/* --- functions --- */
/*
 * _world_node_compare:
 *
 */
static gint
_world_node_compare(
    gconstpointer           a,
    gconstpointer           b
    )
{
    const WorldNode* node1 = a;
    const WorldNode* node2 = b;

    return node1->any.id - node2->any.id;
}

/*
 * _world_node_reset:
 *
 */
static void
_world_node_reset(
    World*                  world
    )
{
    guint i;
    WorldNode* node;

    node = &g_array_index(world->nodes, WorldNode, 0);
    node->any.visited = TRUE;

    for(i = 1; i < world->nodes->len; i++)
    {
        node = &g_array_index(world->nodes, WorldNode, i);
        node->any.visited = FALSE;
    }
}

/*
 * _world_node_draw:
 *
 */
static void
_world_node_draw(
    float4x4*       view,
    WorldNode*      node,
    gint            depth
    )
{
    GList* p;
    WorldNode* portal;
    WorldNode* sculture;

    if(depth <= 0)
    {
        return;
    }

    if(node->any.visited)
    {
        return;
    }

    node->any.visited = TRUE;

    if(!r_frustum_test_bbox(view, node->any.bbox))
        return;
    {
    }

    r_mesh_draw(NULL, node->any.mesh);

    if(node->any.type == WORLD_ROOM)
    {
        for(p = g_list_first(node->room.portals); p != NULL; p = g_list_next(p))
        {
            portal = p->data;
            _world_node_draw(view, portal, depth);
        }
        for(p = g_list_first(node->room.scultures); p != NULL; p = g_list_next(p))
        {
            sculture = p->data;
            if(r_frustum_test_bbox(view, sculture->sculture.bbox))
            {
                r_mesh_draw(NULL, sculture->sculture.mesh);
            }
        }
    }
    else if(node->any.type == WORLD_PORTAL)
    {
        _world_node_draw(view, node->portal.back, depth - 1);
        _world_node_draw(view, node->portal.front, depth - 1);
    }
}

/**
 * world_new:
 *
 **/
World*
world_new(
    RMeshGroup*             groups
    )
{
    World* world;
    WorldNode node;
    gchar* group_name;
    RMesh* group;
    GHashTableIter iter;
    gchar** tokens;
    guint id, id1, id2;

    world = g_slice_new0(World);
    world->nodes = g_array_sized_new(
        FALSE,
        TRUE,
        sizeof(WorldNode),
        g_hash_table_size(groups->groups));
    world->groups = groups;

    id = 0;

    g_hash_table_iter_init(&iter, world->groups->groups);
    while(g_hash_table_iter_next(&iter, (gpointer)&group_name, (gpointer)&group))
    {
        if(!g_str_has_prefix(group_name, "R"))
        {
            continue;
        }

        tokens = g_strsplit(group_name, "_", 0);
        id1 = g_ascii_strtoll(tokens[1], NULL, 10);
        g_strfreev(tokens);

        node.room.id = id1;
        node.room.type = WORLD_ROOM;
        node.room.mesh = group;
        r_mesh_compute_bbox(node.room.mesh, 0, node.room.bbox);
        node.room.portals = NULL;
        node.room.scultures = NULL;
        g_array_append_val(world->nodes, node);

        id++;
    }
    g_array_sort(world->nodes, _world_node_compare);

    g_hash_table_iter_init(&iter, world->groups->groups);
    while(g_hash_table_iter_next(&iter, (gpointer)&group_name, (gpointer)&group))
    {
        if(!g_str_has_prefix(group_name, "P"))
        {
            continue;
        }

        tokens = g_strsplit(group_name, "_", 0);
        id1 = g_ascii_strtoll(tokens[1], NULL, 10);
        id2 = g_ascii_strtoll(tokens[2], NULL, 10);
        g_strfreev(tokens);

        node.portal.id = id;
        node.portal.type = WORLD_PORTAL;
        node.portal.mesh = group;
        r_mesh_compute_bbox(node.portal.mesh, 0, node.portal.bbox);
        node.portal.front = &g_array_index(world->nodes, WorldNode, id1);
        node.portal.back = &g_array_index(world->nodes, WorldNode, id2);
        g_array_append_val(world->nodes, node);

        node.portal.front->room.portals = g_list_prepend(
            node.portal.front->room.portals,
            &g_array_index(world->nodes, WorldNode, id)
            );
        node.portal.back->room.portals = g_list_prepend(
            node.portal.back->room.portals,
            &g_array_index(world->nodes, WorldNode, id)
            );

        id++;
    }

    g_hash_table_iter_init(&iter, world->groups->groups);
    while (g_hash_table_iter_next(&iter, (gpointer)&group_name, (gpointer)&group))
    {
        if(!g_str_has_prefix(group_name, "S"))
        {
            continue;
        }

        tokens = g_strsplit(group_name, "_", 0);
        id1 = g_ascii_strtoll(tokens[1], NULL, 10);
        g_strfreev(tokens);

        node.sculture.id = id;
        node.sculture.type = WORLD_SCULTURE;
        node.sculture.mesh = group;
        r_mesh_compute_bbox(node.sculture.mesh, 0, node.sculture.bbox);
        node.sculture.owner = &g_array_index(world->nodes, WorldNode, id1);
        g_array_append_val(world->nodes, node);

        node.sculture.owner->room.scultures = g_list_prepend(
            node.sculture.owner->room.scultures,
            &g_array_index(world->nodes, WorldNode, id)
            );

        id++;
    }

    return world;
}

/**
 * world_free:
 *
 **/
void
world_free(
    World*                  world
    )
{
    g_array_free(world->nodes, TRUE);
    g_slice_free(World, world);
}

/**
 * world_node_get:
 *
 **/
WorldNode*
world_node_get(
    World*                  world,
    float3*                 position
    )
{
    guint i;
    WorldNode* node;
    float3 p;

    g_assert(world != NULL);
    g_assert(position != NULL);

    for(i = 1; i < world->nodes->len; i++)
    {
        node = &g_array_index(world->nodes, WorldNode, i);

        if(node->any.type == WORLD_SCULTURE)
        {
            return NULL;
        }

        p.x = _ABS(node->any.bbox[0].x - position->x);
        p.y = _ABS(node->any.bbox[0].y - position->y);
        p.z = _ABS(node->any.bbox[0].z - position->z);
        if((p.x <= node->any.bbox[1].x) && (p.y <= node->any.bbox[1].y) && (p.z <= node->any.bbox[1].z))
        {
            return node;
        }
    }

    return NULL;
}

/**
 * world_node_collide:
 *
 **/
gboolean
world_node_collide(
    WorldNode*              node_to_test,
    float3*                 bbox,
    float3*                 reaction
    )
{
    gboolean result = FALSE;
    WorldNode* sculture;
    GList* p;

    g_assert(node_to_test != NULL);
    g_assert(bbox != NULL);
    g_assert(reaction != NULL);

    reaction->x = 0.0f;
    reaction->y = 0.0f;
    reaction->z = 0.0f;

    if(node_to_test->any.type == WORLD_ROOM)
    {
        for(p = g_list_first(node_to_test->room.scultures); p != NULL; p = g_list_next(p))
        {
            sculture = p->data;
            if(r_bbox_overlap(sculture->any.bbox, bbox))
            {
                result |= r_mesh_collide(sculture->any.mesh, 0, bbox, reaction);
            }
        }
    }

    result |= r_mesh_collide(node_to_test->any.mesh, 0, bbox, reaction);
    return result;
}

/**
 * world_node_draw:
 *
 **/
void
world_node_draw(
    float4x4*               view,
    World*                  world,
    WorldNode*              node_to_draw
    )
{
    g_assert(world != NULL);
    g_assert(node_to_draw != NULL);

    if(view != NULL)
    {
        glLoadMatrixf((GLfloat*) view);
    }
    _world_node_reset(world);
    _world_node_draw(view, node_to_draw, 2);
}

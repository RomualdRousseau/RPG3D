/*
 *      mesh.c
 *
 *      Copyright 2008 Romuald Rousseau <romualdrousseau@msn.com>
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
#include "actc.h"

#define CACHE_SIZE 16
#define VBO_OFFSET1(i) ((guchar*)0 + (i))
#define VBO_OFFSET2(i) ((gushort*)0 + (i))
#define VBO_BROKER_UNIT 4

static guint	_last_used_vbo = 0;
static GLuint	_vbo_id[VBO_BROKER_UNIT] = {0};

static gboolean
_r_opengl_mesh_init()
{
	guint i;

	for(i = 0; i < VBO_BROKER_UNIT; i++)
	{
		glGenBuffersARB(1, &_vbo_id[i]);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, _vbo_id[i]);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(RMeshFrame), NULL, GL_STREAM_DRAW_ARB);
	}
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	return TRUE;
}

static void
_r_opengl_mesh_destroy()
{
	guint i;

	for(i = 0; i < VBO_BROKER_UNIT; i++)
	{
		glDeleteBuffersARB(1, &_vbo_id[i]);
	}
}

static inline void
_r_mesh_frame_adjust(
	RFramedMesh*	mesh,
	guint			frame_first,
	guint			frame_last,
	guint			frame_fps,
	guint			time,
	guint*			frame_current,
	RMeshFrame**	frame1,
	RMeshFrame**	frame2
	)
{
	mesh->anim_interpol += 0.000001f * frame_fps * time;
	*frame_current = (guint)mesh->anim_interpol;
	if(*frame_current >= (frame_last - frame_first))
	{
		if(*frame_current > (frame_last - frame_first))
		{
			*frame1 = &mesh->frames[frame_first];
			*frame2 = &mesh->frames[frame_first + 1];
			mesh->anim_interpol = 0.0f;
			*frame_current = 0;
		}
		else
		{
			*frame1 = &mesh->frames[frame_last];
			*frame2 = &mesh->frames[frame_first];
		}
	}
	else
	{
		*frame1 = &mesh->frames[frame_first + *frame_current];
		*frame2 = &mesh->frames[frame_first + *frame_current + 1];
	}
}

static inline void
_r_mesh_frame_lerp(
	RFramedMesh*	mesh,
	RMeshFrame*		frame1,
	RMeshFrame*		frame2,
	gfloat			time,
	RMeshFrame*		result
	)
{
	guint i;

	g_assert(result != NULL);

	if(time > 1.0f)
	{
		time = 1.0f;
	}
	for(i = 0; i < mesh->frames_elements_count; i++)
	{
		lerp3(&frame1->elements[i].vertex, &frame2->elements[i].vertex, time, &result->elements[i].vertex);
		lerp3(&frame1->elements[i].normal, &frame2->elements[i].normal, time, &result->elements[i].normal);
		result->elements[i].texcoord.x = frame1->elements[i].texcoord.x;
		result->elements[i].texcoord.y = frame1->elements[i].texcoord.y;
	}
}

static gboolean
_r_vertex_cache_add(
	gshort*		cache,
	guint		cache_size,
	gshort		value
	)
{
	guint k;

	for(k = 0; (value != cache[k]) && (k < cache_size); k++);
	if(k == cache_size)
	{
		memmove(&cache[1], &cache[0], (cache_size - 1) * sizeof(gshort));
		cache[0] = value;
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

static guint
_r_vertex_cache_test(
	gshort*		cache,
	guint		cache_size,
	gshort		value
	)
{
	guint k;

	for(k = 0; (value != cache[k]) && (k < cache_size); k++);
	if(k == cache_size)
	{
		return 2;
	}
	else if(k > (cache_size - 3))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void
_r_mesh_optimize_triangles(
	RFramedMesh* mesh
	)
{
	gshort cache[CACHE_SIZE];
	guint i, j;
	gshort swp[3];
	gshort* p;
	gshort* q;
	guint s1, s2;

	memset(cache, 0xFE, CACHE_SIZE * sizeof(gshort));
	p = &mesh->triangles[0];
	_r_vertex_cache_add(cache, CACHE_SIZE, p[0]);
	_r_vertex_cache_add(cache, CACHE_SIZE, p[1]);
	_r_vertex_cache_add(cache, CACHE_SIZE, p[2]);
	for(i = 1; i < mesh->triangles_count / 3; i++)
	{
		p = &mesh->triangles[i * 3];
		s1 = 0;
		s1 += _r_vertex_cache_test(cache, CACHE_SIZE, p[0]);
		s1 += _r_vertex_cache_test(cache, CACHE_SIZE, p[1]);
		s1 += _r_vertex_cache_test(cache, CACHE_SIZE, p[2]);

		for(j = i + 1; j < mesh->triangles_count / 3; j++)
		{
			q = &mesh->triangles[j * 3];
			s2 = 0;
			s2 += _r_vertex_cache_test(cache, CACHE_SIZE, q[0]);
			s2 += _r_vertex_cache_test(cache, CACHE_SIZE, q[1]);
			s2 += _r_vertex_cache_test(cache, CACHE_SIZE, q[2]);

			if(s1 > s2)
			{
				memcpy(swp, p, 3 * sizeof(gshort));
				memcpy(p, q, 3 * sizeof(gshort));
				memcpy(q, swp, 3 * sizeof(gshort));
				s1 = s2;
			}
		}

		_r_vertex_cache_add(cache, CACHE_SIZE, p[0]);
		_r_vertex_cache_add(cache, CACHE_SIZE, p[1]);
		_r_vertex_cache_add(cache, CACHE_SIZE, p[2]);
	}
}

static gboolean
_r_mesh_compress_triangles(
	RFramedMesh* mesh
	)
{
	ACTCData *tc;
	guint i;
	guint prim;
	guint v1, v2, v3;
	gshort* tlp;

#ifdef DEBUG
	g_debug("  Frames: %d", mesh->frames_count);
	g_debug("  Elements by Frames: %d", mesh->frames_elements_count);
	g_debug("  triangles: %d", mesh->triangles_count);
#endif

	tc = actcNew();
	if(tc == NULL)
	{
		return FALSE;
	}
	actcParami(tc, ACTC_OUT_HONOR_WINDING, TRUE);

	actcBeginInput(tc);
	for(i = 0; i < mesh->triangles_count; i += 3)
	{
		actcAddTriangle(
			tc,
			mesh->triangles[i + 0],
			mesh->triangles[i + 1],
			mesh->triangles[i + 2]);
	}
	actcEndInput(tc);

	tlp = &mesh->compiled_triangles[0];
	actcBeginOutput(tc);
	while(1)
	{
		prim = actcStartNextPrim(tc, &v1, &v2);
		if(prim == ACTC_DATABASE_EMPTY)
		{
			break;
		}
		i = 0;
		tlp[i++] = 0;
		tlp[i++] = v1;
		tlp[i++] = v2;
		while(actcGetNextVert(tc, &v3) != ACTC_PRIM_COMPLETE)
		{
			tlp[i++] = v3;
		}
		if(prim == ACTC_PRIM_FAN)
		{
			tlp[0] = -(i - 1);
		}
		else
		{
			tlp[0] = +(i - 1);
		}
		tlp += i;
		mesh->compiled_triangles_count += i;
	}
	actcEndOutput(tc);

	actcDelete(tc);

	mesh->compiled_triangles[mesh->compiled_triangles_count++] = 0;

#ifdef DEBUG
	g_debug("  Optimized triangles: %d", mesh->compiled_triangles_count);
#endif

	return TRUE;
}

void
_r_mesh_remap_triangles(
	RFramedMesh* mesh
	)
{
	gshort* p;
	gshort l;
	guint i, j;
	gshort new_indices[4096];
	RVertex new_elements[4096];

	for(i = 0; i < 4096; i++) new_indices[i] = -1;

	p = &mesh->compiled_triangles[0];
	i = 0;
	while((l = *(p++)))
	{
		if(l < 0)
		{
			l = -l;
		}
		for(;l > 0; l--, p++)
		{
			if(new_indices[*p] == -1)
			{
				new_indices[*p] = i++;
			}
		}
	}
	for(i = 0; i < mesh->frames_count; i++)
	{
		for(j = 0; j < mesh->frames_elements_count; j++)
		{
			memcpy(&new_elements[new_indices[j]], &mesh->frames[i].elements[j], sizeof(RVertex));
		}
		memcpy(mesh->frames[i].elements, new_elements, mesh->frames_elements_count * sizeof(RVertex));
	}
	p = &mesh->compiled_triangles[0];
	while((l = *(p++)))
	{
		if(l < 0)
		{
			l = -l;
		}
		for(;l > 0; l--, p++)
		{
			*p = new_indices[*p];
		}
	}

	glGenBuffersARB(1, &mesh->compiled_triangles_vbo);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->compiled_triangles_vbo);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->compiled_triangles_count * sizeof(gshort), mesh->compiled_triangles, GL_STATIC_DRAW_ARB);
	//glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->triangles_count * sizeof(gshort), mesh->triangles, GL_STATIC_DRAW_ARB);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

#ifdef DEBUG
void
_r_mesh_simulate_vertex_cache(
	RFramedMesh* mesh
	)
{
	gshort cache[CACHE_SIZE];
	gshort cache_missed;
	guint vertice = 0;
	guint i;
	gshort* p;

	memset(cache, 0xFE, CACHE_SIZE * sizeof(gshort));
	cache_missed = 0;
	for(i = 0; i < mesh->triangles_count; i++)
	{
		p = &mesh->triangles[i];
		vertice++;
		if(!_r_vertex_cache_add(cache, CACHE_SIZE, *p))
		{
			cache_missed++;
		}
	}
	g_debug("  Simulation: cache missed/vertices: %d/%d", cache_missed, vertice);
}
#endif

#ifdef DEBUG
void
_r_mesh_simulate_vertex_cache_on_compressed_triangles(
	RFramedMesh* mesh
	)
{
	gshort cache[CACHE_SIZE];
	gshort cache_missed = 0;
	guint vertice = 0;
	gshort* p;
	gshort l;

	memset(cache, 0xFE, CACHE_SIZE * sizeof(gshort));
	cache_missed = 0;
	p = &mesh->compiled_triangles[0];
	while((l = *(p++)))
	{
		if(l < 0)
		{
			l = -l;
		}
		for(;l > 0; l--, p++)
		{
			vertice++;
			if(!_r_vertex_cache_add(cache, CACHE_SIZE, *p))
			{
				cache_missed++;
			}
		}
	}
	g_debug("  Simulation: cache missed/vertices: %d/%d", cache_missed, vertice);
}
#endif

RFramedMesh*
r_framed_mesh_new_from_file(
	RMaterial*  	material,
	const gchar*	file_name
	)
{
	RFramedMesh* mesh;
	
	g_assert(file_name != NULL);
	
	_r_opengl_mesh_init();

    mesh = (RFramedMesh*) r_modules_lookup(file_name)->load_from_file(file_name);
	memcpy(&mesh->skin, material, sizeof(RMaterial));
	
#ifdef DEBUG
	_r_mesh_simulate_vertex_cache(mesh);
#endif

	_r_mesh_optimize_triangles(mesh);
	_r_mesh_compress_triangles(mesh);
	_r_mesh_remap_triangles(mesh);

#ifdef DEBUG
	_r_mesh_simulate_vertex_cache_on_compressed_triangles(mesh);
#endif

	return mesh;
}

void
r_framed_mesh_free(
	RFramedMesh*	mesh
	)
{
	g_free(mesh);
	
	_r_opengl_mesh_destroy();
}


void
r_framed_mesh_draw(
	RFramedMesh*	mesh,
	guint			frame_first,
	guint			frame_last,
	guint			frame_fps,
	guint			time
	)
{
	guint frame_current;
	RMeshFrame* frame1;
	RMeshFrame* frame2;
	RMeshFrame* frame;
	gshort i, l;
	gshort* p;

	_r_mesh_frame_adjust(
		mesh,
		frame_first,
		frame_last,
		frame_fps,
		time,
		&frame_current,
		&frame1,
		&frame2
		);

	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->compiled_triangles_vbo);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, _vbo_id[(_last_used_vbo + 1) % VBO_BROKER_UNIT]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(RMeshFrame), NULL, GL_STREAM_DRAW_ARB);
	frame = (RMeshFrame*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB);
	_r_mesh_frame_lerp(
		mesh,
		frame1,
		frame2,
		mesh->anim_interpol - frame_current,
		frame
		);
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

	glVertexPointer(3, GL_FLOAT, sizeof(RVertex), VBO_OFFSET1(0));
	glNormalPointer(GL_FLOAT, sizeof(RVertex), VBO_OFFSET1(12));
	glTexCoordPointer(2, GL_FLOAT, sizeof(RVertex), VBO_OFFSET1(24));

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	if(mesh->skin.texture != 0)
    {
        glBindTexture(GL_TEXTURE_2D, mesh->skin.texture);
        glEnable(GL_TEXTURE_2D);
    }
    else
    {
        glMaterialfv(GL_FRONT, GL_DIFFUSE, (gfloat*)&mesh->skin.color);
    }

	i = 0;
	p = &mesh->compiled_triangles[0];
	while((l = p[i++]))
	{
		if(l < 0)
		{
			l = -l;
			glDrawRangeElements(GL_TRIANGLE_FAN, i, i + l, l, GL_UNSIGNED_SHORT, VBO_OFFSET2(i));
		}
		else
		{
			glDrawRangeElements(GL_TRIANGLE_STRIP, i, i + l, l, GL_UNSIGNED_SHORT, VBO_OFFSET2(i));
		}
		i += l;
	}
	//glDrawRangeElements(GL_TRIANGLES, 0, mesh->triangles_count, mesh->triangles_count, GL_UNSIGNED_SHORT, VBO_OFFSET2(0));

	glDisable(GL_TEXTURE_2D);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

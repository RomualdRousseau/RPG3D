## Makefile.am for rlib

AM_CFLAGS = -Wall -std=c99 -ffast-math -fno-strict-aliasing -march=core2 -mfpmath=sse -DSHM

rlib_public_headers = rlib.h math3d/math3d.h

rlib_c_sources =		\
	utility.c			\
	modules.c			\
	resource_manager.c	\
	desktop_vmode.c		\
	window.c			\
	game.c				\
	renderer.c			\
	renderer_thread.c	\
	renderer_default.c	\
	image.c				\
	texture.c			\
	material.c			\
	mesh.c				\
	surface.c			\
	font.c				\
	console.c

rlib_math3d_c_sources =	\
	math3d/frustum.c	\
	math3d/matrix.c		\
	math3d/collision.c

rlib_modules_c_sources =	\
	modules/tga/tga.c		\
	modules/md2/md2.c		\
	modules/obj/obj.c

include_HEADERS = $(rlib_public_headers)

lib_LTLIBRARIES = librlib.la
librlib_la_SOURCES = $(rlib_c_sources) $(rlib_math3d_c_sources) $(rlib_modules_c_sources) $(rlib_public_headers)

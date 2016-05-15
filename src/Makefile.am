## Makefile.am for RPG

SUBDIRS = rlib

AM_CFLAGS = -I$(srcdir)/rlib -Wall -std=c99 -ffast-math -fno-strict-aliasing -march=core2 -mfpmath=sse -DSHM -DDATADIR=\"$(datadir)\"

rpg_private_headers = globals.h

rpg_c_sources =		\
	main.c			\
	hero.c			\
	world.c		\
	resources.c		\
	render.c		\
	engine.c		\
	physic.c		\
	ai.c

bin_PROGRAMS = rpg
rpg_SOURCES = $(rpg_c_sources) $(rpg_private_headers)
rpg_LDADD = rlib/librlib.la


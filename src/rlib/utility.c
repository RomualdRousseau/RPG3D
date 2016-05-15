/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *      utility.c
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

/* --- functions --- */
/**
 * r_thread_get_cpus_count:
 *
 **/
int
r_thread_get_cpu_count()
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

/**
 * r_thread_set_affinity:
 *
 **/
void
r_thread_set_cpu_affinity(
    gint            cpu
    )
{
    cpu_set_t mask;

#ifdef DEBUG
    g_debug("Set thread %ld(%p): affinity to CPU%d", syscall(SYS_gettid), g_thread_self(), cpu + 1);
#endif
    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);
    sched_setaffinity(syscall(SYS_gettid), sizeof(cpu_set_t), &mask);
}



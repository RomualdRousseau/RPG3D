## configure.ac for test2

AC_INIT([RPG], [1.0], [romualdrousseau@msn.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile src/rlib/Makefile data/Makefile docs/Makefile])

#===
# --enable-debug: build with debugging information (default no)
# Hack! must do this before AC_PROG_CC
#===
AC_ARG_ENABLE(
 debug,
 AC_HELP_STRING([--enable-debug], [build with debugging information (default no)]),,
 enable_debug=no
)
if test "x$enable_debug" = "xyes"; then
 if test "x$CFLAGS" = "x"; then
  CFLAGS="-O0 -g3 -lm"
 fi
 AC_DEFINE(DEBUG, 1, [Define to 1 if we are debugging])
else
 if test "x$CFLAGS" = "x"; then
  CFLAGS="-O3 -lm"
 fi
 AC_DEFINE(G_DISABLE_ASSERT, 1, [Define to 1 if we want to disable glib assertion.])
 AC_DEFINE(G_DISABLE_CHECKS, 1, [Define to 1 if we want to disable glib checks.])
fi

#===
# Setup C compiler
#===
AM_PROG_AR
AC_PROG_CC
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR(m4)
AC_CHECK_HEADER(stdio.h)
AC_CHECK_HEADER(math.h)
AC_DEFINE(_GNU_SOURCE, 1, [Tell the compiler to include GNU extention])
AC_DEFINE(PACKAGE_DATADIR, [DATADIR "/" PACKAGE_NAME], [Define the default data directory.])

#===
# --enable-test-coverage: build with test coverage information (default no)
#===
AC_ARG_ENABLE(
 test_coverage,
 AC_HELP_STRING([--enable-test-coverage], [build with test coverage information (default no)]),,
 enable_test_coverage=no
)
if test "x$enable_test_coverage" = "xyes"; then
 CFLAGS="$CFLAGS -fprofile-arcs -ftest-coverage"
fi

#===
# --enable-fast-math: enable fast math (default yes)
#===
AC_ARG_ENABLE(
 fast_math,
 AC_HELP_STRING([--enable-fast-math], [enable fast math (default yes)]),,
 enable_fast_math=yes
)
if test "x$enable_fast_math" = "xyes"; then
 AC_DEFINE(FASTMATH_SUPPORT, 1, [Define to 1 if we are using fast math])
 CFLAGS="$CFLAGS -ffast-math"
fi

#===
# --enable-back-buffer: use back buffer when rendering (default yes)
#===
AC_ARG_ENABLE(
 back_buffer,
 AC_HELP_STRING([--enable-back-buffer], [use back buffer when rendering (default yes)]),,
 enable_back_buffer=yes
)
if test "x$enable_back_buffer" = "xyes"; then
 AC_DEFINE(BACK_BUFFER, 1, [Define to 1 if we are using back buffer when rendering])
fi

#===
# --enable-vsync: synchronize to vertical trace when rendering (default no)
#===
AC_ARG_ENABLE(
 vsync,
 AC_HELP_STRING([--enable-vsync], [synchronize to vertical trace when rendering (default no)]),,
 enable_vsync=no
)
if test "x$enable_vsync" = "xyes"; then
 AC_DEFINE(VSYNC, 1, [Define to 1 if we are vsync when rendering])
fi

#===
# Check x11 headers
#===
AC_CHECK_HEADER(
 X11/Xlib.h,
 AC_DEFINE(HAVE_XLIB_H, 1, [Define to 1 if you have the <X11/Xlib.h> header file.]),
 AC_MSG_ERROR([Required header <X11/Xlib.h> is missing]),
 [#include <X11/Xlib.h>]
)

#===
# Check xf86dga extension
#===
AC_HAVE_LIBRARY(
 Xxf86dga,
 AC_CHECK_HEADER(
  X11/extensions/Xxf86dga.h,
  [
   AC_DEFINE(HAVE_XF86DGA_H, 1, [Define to 1 if you have the <X11/extensions/Xxf86dga.h> header file.])
   LIBS="$LIBS -lXxf86dga"
  ],
  AC_MSG_ERROR([Required header <X11/extensions/Xxf86dga.h> is missing]),
  [#include <X11/Xlib.h>]
 ),
 AC_MSG_ERROR([Required library Xxf86dga is missing])
)

#===
# Check Xrandr extension
#===
AC_HAVE_LIBRARY(
 Xrandr,
 AC_CHECK_HEADER(
  X11/extensions/Xrandr.h,
  [
   AC_DEFINE(HAVE_XRANDR_H, 1, [Define to 1 if you have the <X11/extensions/Xrandr.h> header file.])
   LIBS="$LIBS -lXrandr"
  ],
  AC_MSG_ERROR([Required header <X11/extensions/Xrandr.h> is missing]),
  [#include <X11/Xlib.h>]
 ),
 AC_MSG_ERROR([Required library Xrandr is missing])
)

#===
# Check OpenGL (GL, GLX, GLU and GLEW)
#===
AC_HAVE_LIBRARY(
 GL,
 [
  AC_CHECK_HEADER(
   GL/gl.h,
   AC_DEFINE(HAVE_GL_H, 1, [Define to 1 if you have the <GL/gl.h> header file.]),
   AC_MSG_ERROR([Required header <GL/gl.h> is missing]),
   [#include <GL/gl.h>]
  )
  AC_CHECK_HEADER(
   GL/glx.h,
   AC_DEFINE(HAVE_GLX_H, 1, [Define to 1 if you have the <GL/glx.h> header file.]),
   AC_MSG_ERROR([Required header <GL/glx.h> is missing]),
   [#include <GL/gl.h>]
  )
  LIBS="$LIBS -lGL"
 ],
 AC_MSG_ERROR([Required library GL is missing])
)
AC_HAVE_LIBRARY(
 GLEW,
 [
  AC_CHECK_HEADER(
   GL/glew.h,
   AC_DEFINE(HAVE_GLEW_H, 1, [Define to 1 if you have the <GL/glew.h> header file.]),
   AC_MSG_ERROR([Required header <GL/glew.h> is missing]),
   [#include <GL/glew.h>]
  )
  LIBS="$LIBS -lGLEW"
 ],
 AC_MSG_ERROR([Required library GLEW is missing])
)

#===
# Check GLib (at least 2.16)
#===
AM_PATH_GLIB_2_0(
 2.16.0,
 [
  AC_DEFINE(HAVE_GLIB_H, 1, [Define to 1 if you have the glib header files.])
  GLIB_INCLUDES=`pkg-config --cflags glib-2.0`
  GLIB_LIBS=`pkg-config --libs glib-2.0 gthread-2.0`
  CFLAGS="$CFLAGS $GLIB_INCLUDES"
  LIBS="$LIBS $GLIB_LIBS"
 ],
 AC_MSG_ERROR([glib 2.16 or newer is required]),
 gthread
)

AC_OUTPUT

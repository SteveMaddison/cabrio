
# Process this file with autoconf to produce a configure script.
AC_INIT(cabrio, 0.8.1, https://groups.google.com/group/cabrio-fe-dev/)

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([build-aux/m4])

AM_INIT_AUTOMAKE([foreign subdir-objects parallel-tests color-tests silent-rules -Wall -Werror])

# Silent Build, sample:
#   CXX    ../cabrio/main.o
AM_SILENT_RULES([yes])


# Information about projet to build packages and the programs.
AC_SUBST([PACKAGE_NAME], ["cabrio"] )
AC_SUBST([PACKAGE_DESCRIPTION_SHORT], ["A graphical front end for emulators, specifically designed for use in arcade cabinets."] )
AC_SUBST([PACKAGE_DESCRIPTION_LONG], ["Cabrio is a graphical front end for emulators, specifically designed for use in arcade cabinets. It allows you to select games via an intuitive, attractive interface which is easy to use with limited controls, such as a joystick. Cabrio is made available under the GNU General Public License, meaning anyone can freely download the source code and even contribute to development."] )
AC_SUBST([PACKAGE_URL], ["http://www.cabrio-fe.org/"] )

AC_SUBST([PACKAGE_AUTHOR], ["Steve Maddison"] )
AC_SUBST([PACKAGE_AUTHOR_EMAIL], ["steve@cosam.org"] )

AC_SUBST([PACKAGE_REQUIRED_BUILD], ["libavcodec-dev libavutil-dev libavformat-dev libglut3-dev libsdl1.2-dev libsdl-image1.2-dev libsdl-gfx1.2-dev libsdl-mixer1.2-dev libsdl-ttf2.0-dev libswscale-dev libxml2-dev zlib1g-dev"] )
AC_SUBST([PACKAGE_REQUIRED_RUNTIME], ["libavcodec libavutil libavformat libglut3 libsdl1.2 libsdl-image1.2 libsdl-gfx1.2 libsdl-mixer1.2 libsdl-ttf2.0 libswscale libxml2 zlib1g"] )

AC_SUBST([PACKAGE_VERSION_MAJOR], [0])
AC_SUBST([PACKAGE_VERSION_MINOR], [8])
AC_SUBST([PACKAGE_VERSION_MINOR_FIX], [1])

AC_SUBST([PACKAGE_VERSION_API], [${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}])


# Checks for programs.
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_CC

# Checks for libraries.

PKG_CHECK_MODULES(libxml2, libxml-2.0)
# SDL
PKG_CHECK_MODULES(SDL, sdl)
PKG_CHECK_MODULES(SDL_ttf, SDL_ttf)
PKG_CHECK_MODULES(SDL_gfx, SDL_gfx)
PKG_CHECK_MODULES(SDL_image, SDL_image)
PKG_CHECK_MODULES(SDL_mixer, SDL_mixer)
# ffmpeg
PKG_CHECK_MODULES(libavformat, libavformat)
PKG_CHECK_MODULES(libavutil, libavutil)
PKG_CHECK_MODULES(libavcodec, libavcodec)
PKG_CHECK_MODULES(libswscale, libswscale)
PKG_CHECK_MODULES(libavdevice, libavdevice)
# openGL
PKG_CHECK_MODULES(GL, gl)
PKG_CHECK_MODULES(GLU, glu)

# Checks for header files.
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([getcwd mkdir socket])


# Set --enable-debug flag.
# If not debug, silent make will be unset.
AC_MSG_CHECKING([Enable debug])
AC_ARG_ENABLE(
	[debug],
	AS_HELP_STRING([--enable-debug],[Turn on debugging]),
	[
	case "${enableval}" in
		yes) debug=true;;
		no)  debug=false ;;
		*)
			AC_MSG_RESULT([${enableval}])
			AC_MSG_ERROR([Bad value ${enableval} for --enable-debug])
		;;
	esac
	],
	[debug=false]
)
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])
AM_COND_IF([DEBUG],
	[
	CPPFLAGS="$CPPFLAGS -g -O0 -DDEBUG"
	CFLAGS="$CFLAGS -g -O0 -DDEBUG"
	LDFLAGS="$LDFLAGS -g -O0 -DDEBUG"

	# Disable silent Build, sample:
	#   CXX    ../cabrio/main.o
	AM_SILENT_RULES([no])
	AC_MSG_RESULT([enable])
],[
	CPPFLAGS="$CPPFLAGS -DNDEBUG"
	CFLAGS="$CFLAGS -DNDEBUG"
	LDFLAGS="$LDFLAGS -DNDEBUG"

	AC_MSG_RESULT([not enable flags])
])

dnl ---------------------------------------------------------------------------
dnl Turn on (almost) all warnings when using gcc
dnl ---------------------------------------------------------------------------
if test "x$GCC" = "xyes"; then
	CPPFLAGS="$CPPFLAGS -Wall"
	## CFLAGS="$CFLAGS --std=c99 -Wall -Wextra -Werror -pedantic"
	CFLAGS="$CFLAGS -Wall"
	## CFLAGS="$CFLAGS -Wno-pointer-sign -Wno-unused -Wno-sign-compare"
	#CFLAGS="$CFLAGS -Wmissing-declarations -Wmissing-prototypes"
	LDFLAGS="$LDFLAGS -Wall"
	AC_SUBST([NO_UNUSED_CFLAGS], [-Wno-unused])
fi

# Search for dlopen dep.
AC_SEARCH_LIBS([dlopen], [dl dld], [],
	[
		AC_MSG_ERROR([unable to find the dlopen() function]
	)
])

# Workaroud to show some vars.
abs_builddir=`pwd`
abs_top_builddir=`pwd`
cd $srcdir

abs_srcdir=`pwd`
abs_top_srcdir=`pwd`
cd $abs_builddir

# Check and expand install path vars.
if test "x${prefix}" = "xNONE"; then
	prefix="${ac_default_prefix}"
fi
if test "x${exec_prefix}" = "xNONE"; then
	exec_prefix="${prefix}"
fi
AC_DEFINE_UNQUOTED([PREFIX], "$(eval echo ${prefix})", [Install prefix directory.])
AC_DEFINE_UNQUOTED([EXEC_PREFIX], ["$(eval echo $exec_prefix)"], [Exec-prefix directory.])
AC_DEFINE_UNQUOTED([LIBDIR], ["$(eval eval echo $libdir)"], [Library directory.] )
AC_DEFINE_UNQUOTED([DATADIR], ["$(eval eval echo $datadir)"], [Data directory.] )
AC_DEFINE_UNQUOTED([PACKAGE_DATA_DIR], ["$(eval eval echo ${datadir}/${PACKAGE_NAME})"], [Data directory.] )
AC_SUBST( libdir, ["$(eval eval echo $libdir)"])
AC_SUBST( includedir, ["$(eval eval echo $includedir)"])
AS_IF([ test "x$pkgsrvdir" = "x" -o "x$pkgsrvdir" = "xNONE" -o "x$pkgsrvdir" = "xnone" ],[
	AC_SUBST( pkgsrvdir, ["$(eval eval echo $localstatedir)/${PACKAGE_NAME}-${PACKAGE_VERSION_MAJOR}"])
])

# Add required deps for linker
# Add required deps for compiler
# Add source code include dir for make. The builddir is add here for de case
# of build in another dir, because the *.in files is generate in the builddir.
CPPFLAGS="$CPPFLAGS -I${abs_top_srcdir}/src"
CFLAGS="$CFLAGS -I${abs_top_srcdir}/src"

CPPFLAGS="$CPPFLAGS -I${abs_top_builddir}/src"
CFLAGS="$CFLAGS -I${abs_top_builddir}/src"

# Override the template file name of the generated .pc file, so that there
# is no need to rename the template file when the API version changes.
AC_CONFIG_FILES([
	Makefile src/Makefile test/Makefile
	packages/debian/control
	cabrio.spec
	])
#	src/config.hpp

AC_CONFIG_HEADERS([src/config.h])

AC_OUTPUT


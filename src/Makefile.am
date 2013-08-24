## Additional flags to pass to aclocal when it is invoked automatically at
## make time. The ${ACLOCAL_FLAGS} variable is picked up from the environment
## to provide a way for the user to supply additional arguments.
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AUTOMAKE_OPTIONS=subdir-objects -Wall -Werror


# The name of program appended the version string.
bin_PROGRAMS = cabrio-@PACKAGE_VERSION_API@



cabrio_@PACKAGE_VERSION_API@_CFLAGS = $(libxml2_CFLAGS)
cabrio_@PACKAGE_VERSION_API@_LDADD = $(libxml2_LIBS)

## Define the source file list for the "@PACKAGE_NAME@-@PACKAGE_VERSION_API@"
## target.  Note that @PACKAGE_VERSION_API@ is not interpreted by Automake and
## will therefore be treated as if it were literally part of the target name,
## and the variable name derived from that.
## Note that it is not necessary to list header files
## which are already listed elsewhere in a _HEADERS variable assignment.
cabrio_@PACKAGE_VERSION_API@_SOURCES = bg.c config.c emulator.c focus.c \
	frame.c game_sel.c key.c lookup.c media.c ogl.c platform.c sdl_wrapper.c \
	snap.c submenu.c category.c control.c event.c font.c game.c hint.c \
	location.c menu.c packet.c sdl_ogl.c setup.c sound.c video.c main.c

dist_data_DATA = data/fonts/ data/pixmaps/ data/sounds/ data/themes/


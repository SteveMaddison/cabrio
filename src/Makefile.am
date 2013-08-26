## Additional flags to pass to aclocal when it is invoked automatically at
## make time. The ${ACLOCAL_FLAGS} variable is picked up from the environment
## to provide a way for the user to supply additional arguments.
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AUTOMAKE_OPTIONS=subdir-objects -Wall -Werror


# The name of program appended the version string.
bin_PROGRAMS = cabrio-@PACKAGE_VERSION_API@



cabrio_@PACKAGE_VERSION_API@_CFLAGS = $(libxml2_CFLAGS) $(SDL_CFLAGS) \
	$(SDL_ttf_CFLAGS) $(SDL_gfx_CFLAGS) $(SDL_image_CFLAGS) \
	$(SDL_mixer_CFLAGS) $(libavdevice_CFLAGS) $(libavformat_CFLAGS) \
	$(libavutil_CFLAGS) $(libavcodec_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS) \
	$(libswscale_CFLAGS)

cabrio_@PACKAGE_VERSION_API@_LDADD = $(libxml2_LIBS) $(SDL_LIBS) \
	$(libavdevice_LIBS) $(SDL_ttf_LIBS) $(SDL_gfx_LIBS) $(SDL_image_LIBS) \
	$(SDL_mixer_LIBS) $(libavformat_LIBS) $(libavutil_LIBS) \
	$(libavcodec_LIBS) $(libswscale_LIBS) $(GL_LIBS) $(GLU_LIBS)


## Define the source file list for the "@PACKAGE_NAME@-@PACKAGE_VERSION_API@"
## target.  Note that @PACKAGE_VERSION_API@ is not interpreted by Automake and
## will therefore be treated as if it were literally part of the target name,
## and the variable name derived from that.
## Note that it is not necessary to list header files
## which are already listed elsewhere in a _HEADERS variable assignment.
cabrio_@PACKAGE_VERSION_API@_SOURCES = bg.c load_config.c emulator.c focus.c \
	frame.c game_sel.c key.c lookup.c media.c ogl.c platform.c sdl_wrapper.c \
	snap.c submenu.c category.c control.c event.c font.c game.c hint.c \
	location.c menu.c packet.c sdl_ogl.c setup.c sound.c video.c main.c

noinst_HEADERS = bg.h load_config.h emulator.h focus.h \
	frame.h game_sel.h key.h lookup.h media.h ogl.h platform.h sdl_wrapper.h \
	snap.h submenu.h category.h control.h event.h font.h game.h hint.h \
	location.h menu.h packet.h sdl_ogl.h setup.h sound.h video.h

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/
	@for f in ${abs_top_srcdir}/src/data/* ; do \
		cp -rf $$f $(DESTDIR)$(pkgdatadir)/ ; \
	done
#fonts/ data/pixmaps/ data/sounds/ data/themes/


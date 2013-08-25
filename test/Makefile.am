## Additional flags to pass to aclocal when it is invoked automatically at
## make time. The ${ACLOCAL_FLAGS} variable is picked up from the environment
## to provide a way for the user to supply additional arguments.
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AUTOMAKE_OPTIONS = subdir-objects parallel-tests -Wall -Werror

#AM_LDFLAGS = -lboost_unit_test_framework $(abs_top_builddir)/src/cabrio-@PACKAGE_VERSION_MAJOR@.la

TESTS = debug.build default_flags_to_configure.build rpm.build simple.build \
	deb.build

XFAIL_TESTS = deb.build rpm.build

TEST_EXTENSIONS = .build
BUILD_LOG_COMPILER = $(SHELL) $(abs_top_srcdir)/test/build_script $(abs_top_srcdir) $(abs_top_builddir)


SUBDIRS=src/ test

## Place generated object files (.o) into the same directory as their source
## files, in order to avoid collisions when non-recursive make is used.
AUTOMAKE_OPTIONS = subdir-objects

## Additional flags to pass to aclocal when it is invoked automatically at
## make time. The ${ACLOCAL_FLAGS} variable is picked up from the environment
## to provide a way for the user to supply additional arguments.
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

## Install the generated pkg-config file (.pc) into the expected location for
## architecture-dependent package configuration information.  Occasionally,
## pkg-config files are also used for architecture-independent data packages,
## in which case the correct install location would be $(datadir)/pkgconfig.
##pkgconfigdir = $(libdir)/pkgconfig
##pkgconfig_DATA = cabrio-$(PACKAGE_VERSION_MAJOR).pc

## Define an independent executable script for inclusion in the distribution
## archive.  However, it will not be installed on an end user's system due to
## the noinst_ prefix.
dist_noinst_SCRIPTS = bootstrap.sh


TEMP_PKG_RPM_DIR=$(abs_top_builddir)/packages/rpm
TEMP_PKG_DEB_DIR=$(abs_top_builddir)/packages/debian/root

# Create a deb package
pkg-deb:
	mkdir -p $(TEMP_PKG_DEB_DIR)/DEBIAN

	make DESTDIR=$(TEMP_PKG_DEB_DIR) install
	cp packages/debian/control $(TEMP_PKG_DEB_DIR)/DEBIAN/control
	echo -n 'Depends: ' >> $(TEMP_PKG_DEB_DIR)/DEBIAN/control
	find $(TEMP_PKG_DEB_DIR) -not -type d | xargs -n1 objdump -p 2>/dev/null \
		| grep 'NEEDED ' |tr -s ' ' |cut -f 3 -d ' ' | \
		xargs -n1 dpkg -S |cut -f1 -d: |sort -u | xargs echo | tr ' ' ','  >> $(TEMP_PKG_DEB_DIR)/DEBIAN/control
	dpkg --build $(TEMP_PKG_DEB_DIR) $(TEMP_PKG_DEB_DIR)/../$(PACKAGE_NAME)-$(PACKAGE_VERSION_API).deb
	- rm -r $(TEMP_PKG_DEB_DIR)

# Create an RPM package
# Depend of all rule
pkg-rpm: all
	mkdir -p $(TEMP_PKG_RPM_DIR)/RPMS/i386
	mkdir -p $(TEMP_PKG_RPM_DIR)/SRPMS
	mkdir -p $(TEMP_PKG_RPM_DIR)/BUILD
	mkdir -p $(TEMP_PKG_RPM_DIR)/SOURCES
	mkdir -p $(TEMP_PKG_RPM_DIR)/tmp
	@- rm -r $(TEMP_PKG_RPM_DIR)/BUILD/$(PACKAGE_NAME)-root
	@- ln -s $(abs_top_builddir) $(TEMP_PKG_RPM_DIR)/BUILD/$(PACKAGE_NAME)-root
	@- rm -r $(TEMP_PKG_RPM_DIR)/RPMS/$(PACKAGE_NAME)-*
	@- rm -r $(TEMP_PKG_RPM_DIR)/SRPMS/$(PACKAGE_NAME)-*
	@- rm -r $(TEMP_PKG_RPM_DIR)/SOURCES/$(PACKAGE_NAME)-*

#	Find binary deps for binary in $(abs_top_builddir) to write in rpm
#	deps in cabrio.spec
	sed -i s/"^Requires: .*/Requires: ` ( find $(abs_top_builddir) -not -type d \
	| xargs -n1 objdump -p | grep 'NEEDED ' \
	| tr -s ' ' |cut -f 3 -d ' ' | sort -u \
	| xargs -n 1 printf -- " -e %s" \
	| xargs -I'{}' bash -c 'rpm -q -a --filesbypkg | grep {}' \
	| tr -s ' ' | cut -f1 -d ' ' | sort -u \
	| xargs echo | tr ' ' ',' ) 2> /dev/null `/"g $(abs_top_builddir)/$(PACKAGE_NAME).spec

	- rpmbuild --with noconfigure --quiet --nodeps --define="_topdir $(TEMP_PKG_RPM_DIR)" \
		-bb $(abs_top_builddir)/$(PACKAGE_NAME).spec
	mv $(TEMP_PKG_RPM_DIR)/RPMS/i386/$(PACKAGE_NAME)-$(PACKAGE_VERSION_API)-*.i386.rpm $(TEMP_PKG_RPM_DIR)/../


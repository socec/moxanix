# components
MOXERVER = moxerver
MOXANIX = moxanix

# ==============================================================================

# system install root directory
INSTALL_ROOT = ./install.dir

# prefix for /bin directory
BIN_PREFIX = /usr

# ==============================================================================

# directories used for local component builds
BUILDDIR = build.dir
INSTALLDIR = install.dir

# ==============================================================================

# supported make options (clean, install...)
.PHONY: all default install clean

# all calls all other options
all: default install

# default builds components
default:
	cd $(MOXERVER) && make BUILDDIR=$(BUILDDIR) INSTALLDIR=$(INSTALLDIR)
	cd $(MOXANIX) && make BUILDDIR=$(BUILDDIR) INSTALLDIR=$(INSTALLDIR)

# install handles component installation
install: default
	mkdir -p $(INSTALL_ROOT)

	cd $(MOXERVER) && make install BUILDDIR=$(BUILDDIR) INSTALLDIR=$(INSTALLDIR) BIN_PREFIX=$(BIN_PREFIX)
	cp -r $(MOXERVER)/$(INSTALLDIR)/* $(INSTALL_ROOT)/

	cd $(MOXANIX) && make install BUILDDIR=$(BUILDDIR) INSTALLDIR=$(INSTALLDIR) BIN_PREFIX=$(BIN_PREFIX)
	cp -r $(MOXANIX)/$(INSTALLDIR)/* $(INSTALL_ROOT)/

# clean removes build and install results
clean:
	cd $(MOXERVER) && make clean BUILDDIR=$(BUILDDIR) INSTALLDIR=$(INSTALLDIR)
	cd $(MOXANIX) && make clean BUILDDIR=$(BUILDDIR) INSTALLDIR=$(INSTALLDIR)
	-rm -rf $(INSTALL_ROOT)

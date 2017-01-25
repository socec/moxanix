# components
SERVER = moxerver
TOOLS = tools

# ==============================================================================

# system install root directory
INSTALL_ROOT = ./install.dir

# prefix for /bin directory
BIN_PREFIX = /usr

# ==============================================================================

# directories used for local component builds
BUILDDIR = build.dir
INSTALLDIR = install.dir
# directory configuration for local component builds
DIR_CONFIG = BUILDDIR=$(BUILDDIR) INSTALLDIR=$(INSTALLDIR) BIN_PREFIX=$(BIN_PREFIX)

# ==============================================================================

# supported make options (clean, install...)
.PHONY: all default install clean

# all calls all other options
all: default install

# default builds components
default:
	cd $(SERVER) && make $(DIR_CONFIG)
	cd $(TOOLS) && make $(DIR_CONFIG)

# install handles component installation
install: default
	mkdir -p $(INSTALL_ROOT)

	cd $(SERVER) && make install $(DIR_CONFIG)
	cp -r $(SERVER)/$(INSTALLDIR)/* $(INSTALL_ROOT)/

	cd $(TOOLS) && make install $(DIR_CONFIG)
	cp -r $(TOOLS)/$(INSTALLDIR)/* $(INSTALL_ROOT)/

# clean removes build and install results
clean:
	cd $(SERVER) && make clean $(DIR_CONFIG)
	cd $(TOOLS) && make clean $(DIR_CONFIG)
	-rm -rf $(INSTALL_ROOT)

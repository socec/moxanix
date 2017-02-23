# components
SERVER = moxerver
CONTROL = moxerverctl

# system install root directory
INSTALL_ROOT = $(abspath /)

# prefix for /bin directory
BIN_PREFIX = /usr

# ==============================================================================

# directories used for local component builds
BUILDDIR = build.dir
# directory configuration for local component builds
DIR_CONFIG = BUILDDIR=$(BUILDDIR) INSTALLDIR=$(abspath $(INSTALL_ROOT)) BIN_PREFIX=$(BIN_PREFIX)

# ==============================================================================

# supported make options (clean, install...)
.PHONY: default install clean

# default builds components
default:
	cd $(SERVER) && make $(DIR_CONFIG)
	cd $(CONTROL) && make $(DIR_CONFIG)

# install handles component installation
install: default
	mkdir -p $(INSTALL_ROOT)
	cd $(SERVER) && make install $(DIR_CONFIG)
	cd $(CONTROL) && make install $(DIR_CONFIG)

# clean removes build and install results
clean:
	cd $(SERVER) && make clean $(DIR_CONFIG)
	cd $(CONTROL) && make clean $(DIR_CONFIG)
	-rm -rf $(INSTALL_ROOT)

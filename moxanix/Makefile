# target names
TARGET_CONTROL = moxanix.sh
TARGET_CONFIG = moxanix.cfg

# ==============================================================================

# directory for build results
BUILDDIR = build.dir
# installation root
INSTALLDIR = install.dir
# prefix for /bin directory
BIN_PREFIX = /usr

# ==============================================================================

# supported make options (clean, install...)
.PHONY: default install clean

# default does nothing
default:

# install targets
install:
	# install control script
	mkdir -p $(INSTALLDIR)/$(BIN_PREFIX)/bin
	cp $(TARGET_CONTROL) $(INSTALLDIR)/$(BIN_PREFIX)/bin/
	# install configuration file
	mkdir -p $(INSTALLDIR)/etc
	cp $(TARGET_CONFIG) $(INSTALLDIR)/etc/

# clean removes object files and target (ignore errors with "-" before commands)
clean:
	-rm -rf $(INSTALLDIR)

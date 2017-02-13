# target names
TARGET_CONTROL = moxerverctl
TARGET_CONFIG = moxerver.cfg

# ==============================================================================

# directory for build results
BUILDDIR = $(abspath build.dir)
# installation root
INSTALLDIR = $(abspath install.dir)
# prefix for /bin directory
BIN_PREFIX = /usr

# ==============================================================================

# supported make options (clean, install...)
.PHONY: default install clean

# default does nothing
default:

# install targets
install:
	# install configuration file
	install -Dm0644 $(TARGET_CONFIG) $(INSTALLDIR)/etc/$(TARGET_CONFIG)
	# install control script, referring to the configuration file
	install -Dm0755 $(TARGET_CONTROL) $(INSTALLDIR)/$(BIN_PREFIX)/bin/$(TARGET_CONTROL)
	sed -i -e 's#ROOT=\"\"#ROOT=$(INSTALLDIR)#' $(INSTALLDIR)/$(BIN_PREFIX)/bin/$(TARGET_CONTROL)

# clean removes object files and target (ignore errors with "-" before commands)
clean:
	-rm -rf $(INSTALLDIR)

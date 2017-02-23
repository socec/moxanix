# target names
TARGET_CONTROL = moxerverctl
TARGET_CONFIG = moxerver.cfg
TARGET_SERVICE = moxerver.service

# ==============================================================================

# directory for build results
BUILDDIR = $(abspath build.dir)
# installation root
INSTALLDIR = $(abspath install.dir)
# user prefix, used in front of /bin and /lib directories
USER_PREFIX = usr

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
	install -Dm0755 $(TARGET_CONTROL) $(INSTALLDIR)/$(USER_PREFIX)/bin/$(TARGET_CONTROL)
	sed -i -e 's#ROOT=\"\"#ROOT=$(INSTALLDIR)#' $(INSTALLDIR)/$(USER_PREFIX)/bin/$(TARGET_CONTROL)
	# install systemd service file
	install -Dm0755 $(TARGET_SERVICE) $(INSTALLDIR)/$(USER_PREFIX)/lib/systemd/system/$(TARGET_SERVICE)
	sed -i -e 's#/usr/bin#$(INSTALLDIR)/$(USER_PREFIX)/bin#' $(INSTALLDIR)/$(USER_PREFIX)/lib/systemd/system/$(TARGET_SERVICE)

# clean removes object files and target (ignore errors with "-" before commands)
clean:
	-rm -rf $(INSTALLDIR)/etc/$(TARGET_CONFIG)
	-rm -rf $(INSTALLDIR)/$(USER_PREFIX)/bin/$(TARGET_CONTROL)
	-rm -rf $(INSTALLDIR)/$(USER_PREFIX)/lib/systemd/system/$(TARGET_SERVICE)

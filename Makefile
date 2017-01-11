# components
MOXERVER = moxerver
MOXANIX = moxanix

# installation root
INSTALL_ROOT = ./install.dir

# ==============================================================================

MOXERVER_BUILDDIR = build.dir

# ==============================================================================

# supported make options (clean, install...)
.PHONY: default all clean

# all calls all other options
all: default

# default builds moxerver
default:
	cd $(MOXERVER) && make OUTDIR=$(MOXERVER_BUILDDIR)

# install handles moxerver and moxanix installation
install: default
	mkdir -p $(INSTALL_ROOT)
	cp $(MOXERVER)/$(MOXERVER_BUILDDIR)/$(MOXERVER) $(INSTALL_ROOT)/$(MOXERVER)
	cp $(MOXANIX)/$(MOXANIX).* $(INSTALL_ROOT)/

# clean removes build and install results
clean:
	cd $(MOXERVER) && make clean
	-rm -rf $(INSTALL_ROOT)

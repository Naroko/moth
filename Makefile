# Scratch directory for building extrenal sources
BUILD = build

# Root to install things before they're packaged
TARGET = target

# Downloaded source files go here
CACHE = cache

# The end result
BIN = bin


all: packages

dist: ctf-install.zip
ctf-install.zip: packages.zip /usr/lib/syslinux/mbr.bin
	zip --junk-paths $@ packages.zip /usr/lib/syslinux/mbr.bin install.sh

packages.zip: packages bzImage rootfs.squashfs
	zip --junk-paths $@ bin/*.pkg bzImage rootfs.squashfs

clean: packages-clean
	rm -rf $(BUILD) $(TARGET) $(BIN)

scrub: clean
	rm -rf $(CACHE)

-include */*.mk
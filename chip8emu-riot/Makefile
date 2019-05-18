APPLICATION = chip8emu-riot
BOARD ?= native
RIOTBASE ?= $(CURDIR)/../RIOT

DEVELHELP ?= 1

USEPKG += u8g2

USEMODULE += fmt
USEMODULE += newlib

# threading
USEMODULE += xtimer
USEMODULE += random
USEMODULE += posix_headers
USEMODULE += pthread

include Makefile.$(BOARD)

# filesystem
CFLAGS += -DVFS_FILE_BUFFER_SIZE=72 -DVFS_DIR_BUFFER_SIZE=44
USEMODULE += vfs
# USEMODULE += devfs
# USEMODULE += constfs
# USEMODULE += spiffs
USEMODULE += fatfs_vfs
FATFS_IMAGE_FILE_SIZE_MIB ?= 128

# shell
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# chip8emulator specific
CFLAGS += -DCHIP8EMU_NO_THREAD

include $(RIOTBASE)/Makefile.include

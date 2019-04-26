APPLICATION = chip8emu-riot
BOARD ?= native
RIOTBASE ?= $(CURDIR)/../RIOT

DEVELHELP ?= 1

USEMODULE += posix_headers
USEMODULE += pthread
USEMODULE += xtimer
USEMODULE += random

CFLAGS += -DCHIP8EMU_NO_THREAD
include $(RIOTBASE)/Makefile.include

APPLICATION = chip8emu-riot
BOARD ?= native
RIOTBASE ?= $(CURDIR)/../RIOT


DEVELHELP ?= 1

# QUIET ?= 0

USEPKG += u8g2

ifeq ($(BOARD),native)
	USEMODULE += u8g2_sdl
	CFLAGS += -DNATIVE_BOARD
else
	FEATURES_REQUIRED = periph_gpio
	FEATURES_REQUIRED = periph_i2c
endif
# USEMODULE += u8g2_utf8

USEMODULE += xtimer
USEMODULE += random
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += vfs
USEMODULE += devfs
#USEMODULE += littlefs
# USEMODULE += constfs


# Set file systems specific variables
# ifneq (,$(filter littlefs, $(USEMODULE)))
#     CFLAGS += -DVFS_FILE_BUFFER_SIZE=56 -DVFS_DIR_BUFFER_SIZE=44
# else ifneq (,$(filter spiffs, $(USEMODULE)))
#     SPIFFS_NB_FD ?= 8
#     CFLAGS += '-DSPIFFS_FS_FD_SPACE_SIZE=(32 * $(SPIFFS_NB_FD))'
# endif

CFLAGS += -DCHIP8EMU_NO_THREAD

include $(RIOTBASE)/Makefile.include

APPLICATION = chip8emu-riot
BOARD ?= native
RIOTBASE ?= $(CURDIR)/../RIOT

DEVELHELP ?= 1


USEPKG += u8g2

ifeq ($(BOARD),native)
	USEMODULE += u8g2_sdl
	CFLAGS += -DNATIVE_BOARD
else
	FEATURES_REQUIRED += periph_gpio
	FEATURES_REQUIRED += periph_i2c
	FEATURES_OPTIONAL += periph_rtc
endif

USEMODULE += fmt

# threading
USEMODULE += xtimer
USEMODULE += random
USEMODULE += posix_headers
USEMODULE += pthread

# filesystem
CFLAGS += -DVFS_FILE_BUFFER_SIZE=72 -DVFS_DIR_BUFFER_SIZE=44
USEMODULE += vfs
# USEMODULE += devfs
# USEMODULE += constfs
# USEMODULE += spiffs
USEMODULE += fatfs_vfs
FATFS_IMAGE_FILE_SIZE_MIB ?= 128

USEMODULE += sdcard_spi
USEMODULE += mtd_sdcard
USEMODULE += auto_init_storage
CFLAGS += -DSDCARD_SPI_PARAM_CS=GPIO_PIN\(PORT_B,6\)
CFLAGS += -DSDCARD_SPI_PARAM_CLK=GPIO_PIN\(PORT_A,5\)
CFLAGS += -DSDCARD_SPI_PARAM_MISO=GPIO_PIN\(PORT_A,6\)
CFLAGS += -DSDCARD_SPI_PARAM_MOSI=GPIO_PIN\(PORT_A,7\)

# shell
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# chip8emulator specific
CFLAGS += -DCHIP8EMU_NO_THREAD

include $(RIOTBASE)/Makefile.include

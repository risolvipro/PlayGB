HEAP_SIZE      = 8388208
STACK_SIZE     = 61800

PRODUCT = PlayGB.pdx

SDK = ${PLAYDATE_SDK_PATH}
ifeq ($(SDK),)
	SDK = $(shell egrep '^\s*SDKRoot' ~/.Playdate/config | head -n 1 | cut -c9-)
endif

ifeq ($(SDK),)
$(error SDK path not found; set ENV value PLAYDATE_SDK_PATH)
endif

VPATH += src
VPATH += peanut_gb
VPATH += minigb_apu

# List C source files here
SRC += minigb_apu/minigb_apu.c

SRC += main.c
SRC += src/app.c
SRC += src/utility.c
SRC += src/scene.c
SRC += src/library_scene.c
SRC += src/game_scene.c
SRC += src/array.c
SRC += src/listview.c
SRC += src/preferences.c

ASRC = setup.s

# List all user directories here
UINCDIR += src
UINCDIR += peanut_gb
UINCDIR += minigb_apu
UINCDIR += lcd

# List all user C define here, like -D_DEBUG=1
UDEFS = 

# Define ASM defines here
UADEFS = 

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

include $(SDK)/C_API/buildsupport/common.mk

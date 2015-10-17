######################################
## CS1372 Dual GBA/Console Makefile ##
## Updated: 9/12/2010 - Kyle Lemons ##
######################################
# Student instructions:
# - Ensure that the paths are correct
# - Edit the SOURCES line with a space-separated list of your .c files
# - Keep updating SOURCES every time you add a new source file
# - Select "Release" in NetBeans if this is a GBA project
# - Select "Debug" in NetBeans if this is a console project
# - Make sure the Linker Output in NetBeans is set to "cs1372" for all configurations
# - I recommend setting the Run Console Type to Output Window
# Grading instructions:
# - Ensure that the paths are correct
# - Remove the SOURCES line above
# - Add one of the following two lines before the ifeq line (no indent):
#   CONF = Debug
#   CONF = Release

# --- Project Settings (Change these for your proejct)
# PRODUCT_NAME should match your Linker Output in NetBeans
# SOURCES should be The .c files in your project (except in the autograder)
PRODUCT_NAME       = cs2261
SOURCES            = main.c myLib.c text.c Pokemon_RBY_PalletTown.c sprites.c collision.c floatingText.c walk.c opening2.c gameMusic.c victory.c collide.c defeat.c

# --- System Settings (Update these for your system)
# DKPATH should contain arm-eabi-gcc{.exe}
# CCPATH should contain gcc{.exe}
# VBASIM should point to your VBA executable (or open on mac)
DKPATH             = C:/devkitARM/bin
CCPATH             = C:/cygwin/bin
VBASIM             = C:/vba/VisualBoyAdvance.exe
FIND               = find
COPY               = cp -r

# --- File Names
ELF_NAME           = $(PRODUCT_NAME).elf
ROM_NAME           = $(PRODUCT_NAME).gba
BIN_NAME           = $(PRODUCT_NAME)

# --- Debug and Release Selection
# Don't change this or it'll break your Makefile
# If you need to override the selection, uncomment one:
#CONF               = Debug
#CONF               = Release
ifeq ($(CONF),Debug)
DEBUG              = yes
endif

ifndef DEBUG
# ============ RELEASE MODE
# --- GBA Settings
MODEL              = -mthumb-interwork -marm -mlong-calls
SPECS              = -specs=gba.specs 

# --- Archiver
AS                 = $(DKPATH)/arm-eabi-as
ASFLAGS            = -mthumb-interwork

# --- Compiler
CC                 = $(DKPATH)/arm-eabi-gcc
CFLAGS             = $(MODEL) -O2 -Wall -pedantic -Wextra -D_ROM=$(ROM_NAME) -D_VBA=$(VBASIM)
CC_WRAP            = $(CCPATH)/gcc
CFLAGS_WRAP        = -O3 -Wall -pedantic -D_ROM='"$(ROM_NAME)"' -D_VBA='"$(VBASIM)"'

# --- Linker
LD                 = $(DKPATH)/arm-eabi-gcc
LDFLAGS            = $(SPECS) $(MODEL) -lm

# --- Object/Executable Packager
OBJCOPY            = $(DKPATH)/arm-eabi-objcopy
OBJCOPYFLAGS       = -O binary

# --- ROM Fixer
GBAFIX             = $(DKPATH)/gbafix

# --- Delete
RM                 = rm -f

OBJECTS = $(filter-out gba_wrapper%,$(SOURCES:.c=.o))

# --- Main build target
all : build $(BIN_NAME)

run : build
	$(VBASIM) $(ROM_NAME)

build : UNZIP $(ROM_NAME)

$(BIN_NAME) : gba_wrapper.c
	$(CC_WRAP) $(CFLAGS_WRAP) -o $@ $^

# --- Build .elf file into .gba ROM file
$(ROM_NAME) : $(ELF_NAME)
	$(OBJCOPY) $(OBJCOPYFLAGS) $(ELF_NAME) $(ROM_NAME)
	$(GBAFIX) $(ROM_NAME)

# --- Build .o files into .elf file
$(ELF_NAME) : $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@

# -- Build .c files into .o files
$(OBJECTS) : %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ============ RELEASE MODE
else
# ============ DEBUG MODE
# --- Compiler
CC                 = $(CCPATH)/gcc
CFLAGS             = -D_DEBUG -Wall -pedantic -Wextra -ggdb

all : build

run : build
	./$(BIN_NAME)

clean :

build : UNZIP $(BIN_NAME)

$(BIN_NAME) : $(SOURCES)
	$(CC) -o $(BIN_NAME) $(CFLAGS) $(SOURCES)

# ============ DEBUG MODE
endif

# ============ Common
UNZIP :
	-@$(FIND) . -iname "*.zip" -exec unzip -n {} \; -exec echo "This project must be rebuilt" \; -exec rm {} \;

clean:
	$(RM) $(ROM_NAME)
	$(RM) $(ELF_NAME)
	$(RM) $(BIN_NAME)
	$(RM) *.o
# ============ Common

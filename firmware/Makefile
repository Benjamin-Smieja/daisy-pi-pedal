# Project Name
TARGET = daisy-pi-pedal-firmware

SRC_DIR = src

# List just the clean filenames
FILES = main.cpp \
        effect.cpp \
        trem.cpp \
        rpiComsManager.cpp \
        effectChain.cpp

# Make automatically combines them into: src/main.cpp
CPP_SOURCES = $(addprefix $(SRC_DIR)/, $(FILES))

C_INCLUDES = -I../protocol

# Library Locations
DAISYSP_DIR ?= lib/DaisySP
LIBDAISY_DIR ?= lib/libDaisy

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile


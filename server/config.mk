# C compiler on your system, "cc" or "gcc" are usually good choices
CC := gcc

# "make" command, usually just "make"
MAKE := make

# Installation directory
PREFIX := $(shell conda info --base)/envs/$(shell conda env list | grep "*" | cut -f1 -d ' ')

JANSSON_INC := $(PREFIX)/include
JANSSON_LIB := $(PREFIX)/lib

# Man pages directory
MANDIR := ${PREFIX}/share/man

# C compilation flags
CFLAGS := ${CFLAGS} -I$(JANSSON_INC) -std=c99 -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=500 -Wall -pedantic -Wno-error -fPIE -fno-strict-aliasing -pthread
# CFLAGS := ${CFLAGS} -I${PREFIX}/include

# C linker flags
LDFLAGS := ${LDFLAGS} -pie -pthread -L$(JANSSON_LIB)/lib
# LD_LIBRARY_PATH = $(CONDA_PREFIX)/lib
# LDFLAGS := ${LDFLAGS} -L${PREFIX}/lib -ljansson -lev -lssl -lcrypto -lz

# Uncomment the lines below for a release build
#CFLAGS += -DNDEBUG -O3

# Uncomment the lines below for a debug build
CFLAGS += -Og -g -fstack-protector-strong

# Uncomment the lines below to enable AddressSanitizer (Valgrind will not work)
#CFLAGS += -fsanitize=address
#LDFLAGS += -fsanitize=address

# Uncomment the lines below to compile for Windows
#CC := x86_64-w64-mingw32-gcc
#LDFLAGS += -lws2_32

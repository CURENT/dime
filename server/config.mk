# C compiler on your system, "cc" or "gcc" are usually good choices
CC ?= cc

# "make" command, usually just "make"
MAKE ?= make

# Installation directory
PREFIX ?= /usr/local

# Man pages directory
MANDIR ?= ${PREFIX}/share/man

# C compilation flags
CFLAGS ?= -std=c99 -D_XOPEN_SOURCE=500 -Wall -pedantic -Wno-error -fPIE -pthread

#C linker flags
LDFLAGS ?= -pie -pthread -ljansson

# Uncomment the lines below for a release build
#CFLAGS += -DNDEBUG -O3

# Uncomment the lines below for a debug build
CFLAGS += -Og -g -fstack-protector-strong -fstack-clash-protection

# Uncomment the lines below to enable AddressSanitizer (Valgrind will not work)
#CFLAGS += -fsanitize=address
#LDFLAGS += -fsanitize=address

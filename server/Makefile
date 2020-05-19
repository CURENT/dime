CC ?= cc
MAKE ?= make
PREFIX ?= /usr/local
MANDIR ?= ${PREFIX}/share/man

CFLAGS ?= -std=c99 -D_XOPEN_SOURCE=500 -Wall -pedantic -Wno-error -fPIE
LDFLAGS ?= -pie -ljansson

# Uncomment the lines below for a release build
#CFLAGS += -DNDEBUG -O3
# Uncomment the lines below for a debug build
CFLAGS += -Og -g -fstack-protector-strong -fstack-clash-protection

SRCS = deque.c main.c ringbuffer.c server.c socket.c table.c
OBJS = ${SRCS:.c=.o}

%.o: %.c
	${CC} $< ${CFLAGS} -c -o $@

dime: ${OBJS}
	${CC} ${OBJS} ${LDFLAGS} -o $@

all: dime

install: all
	install -s dime ${PREFIX}/bin

clean:
	rm -f dime ${OBJS}

.PHONY: all clean

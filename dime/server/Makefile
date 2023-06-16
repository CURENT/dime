include config.mk

SRCS = deque.c client.c main.c log.c ringbuffer.c server.c socket.c table.c
OBJS = ${SRCS:.c=.o}

%.o: %.c
	${CC} $< ${CFLAGS} -c -o $@

dime: ${OBJS}
	${CC} ${OBJS} -o $@ -ljansson -lev -lssl -lcrypto -lz ${LDFLAGS}

all: dime

install: all
	install -s dime ${PREFIX}/bin

clean:
	rm -f dime ${OBJS}

.PHONY: all install clean

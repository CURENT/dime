include config.mk
# Print CFLAGS and LDFLAGS
$(info LDFLAGS: $(LDFLAGS))

SRCS = deque.c client.c main.c log.c ringbuffer.c server.c socket.c table.c
OBJS = ${SRCS:.c=.o}

%.o: %.c
	${CC} $< ${CFLAGS} -c -o $@

dimes: ${OBJS}
	${CC} ${OBJS} -o $@ -lev -ljansson -lssl -lcrypto -lz ${LDFLAGS}

all: dimes

install: all
	install -s dimes ${PREFIX}/bin

clean:
	rm -f dimes ${OBJS}

.PHONY: all install clean

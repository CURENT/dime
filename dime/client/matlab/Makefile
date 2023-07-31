include config.mk

SRCS = sunclose.c sunconnect.c sunrecv.c sunsend.c
OBJS = ${SRCS:.c=.${EXT}}

%.${EXT}: %.c
	${MEX} $< -output $@

all: ${OBJS}

install: all
	install sunclose.${EXT} ${MATLABPATH}
	install sunconnect.${EXT} ${MATLABPATH}
	install sunrecv.${EXT} ${MATLABPATH}
	install sunsend.${EXT} ${MATLABPATH}
	install -m 0644 dime.m ${MATLABPATH}
	install -m 0644 dimebloads.m ${MATLABPATH}
	install -m 0644 dimebdumps.m ${MATLABPATH}

clean:
	rm -f ${OBJS}

.PHONY: all install clean

# This may need to be changed depending on your OS
EXT = mexa64

SRCS = sunconnect.c sunsend.c sunrecv.c sunclose.c
OBJS = ${SRCS:.c=.${EXT}}

%.${EXT}: %.c
	mex $< -output $@

all: ${OBJS}

clean:
	rm -f ${OBJS}

.PHONY: all clean

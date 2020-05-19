MATLABPATH ?= ${HOME}/Documents/MATLAB

# This may need to be changed depending on your OS
EXT = mexa64

SRCS = sunclose.c sunconnect.c sunrecv.c sunsend.c
OBJS = ${SRCS:.c=.${EXT}}

%.${EXT}: %.c
	mex $< -output $@

all: ${OBJS}

install: all
	install sunclose.mexa64 ${MATLABPATH}
	install sunconnect.mexa64 ${MATLABPATH}
	install sunrecv.mexa64 ${MATLABPATH}
	install sunsend.mexa64 ${MATLABPATH}
	install -m 0644 dime.m ${MATLABPATH}

clean:
	rm -f ${OBJS}

.PHONY: all install clean

FROM alpine

COPY . /tmp
WORKDIR /tmp/server

RUN apk update
RUN apk add gcc make libc-dev jansson-dev
RUN make clean
RUN make
RUN make install

EXPOSE 5000

ENTRYPOINT ["/usr/local/bin/dime"]
CMD ["-P", "tcp", "-p", "5000"]

# ------------------------------------------------------------------------------
# Makefile for sample networking programs
# ------------------------------------------------------------------------------

CC = gcc
CFLAGS = -g -D_REENTRANT -Wall -D__EXTENSIONS__

# what's the OS type?
UNAME = $(shell uname)

ifeq ($(UNAME), SunOS)
LIBS = -lresolv -lsocket -lnsl
endif

ifeq ($(UNAME), Linux)
LIBS = -lresolv -lnsl 
endif

# directory containing .h files
INC   = include

# files to be cleaned
CLEAN = core *.o *.out typescript* *~ *dSYM

# common object files
COBJS = bencode.o connections.o robust_io.o chatty_util_functions.o sha1.o color.o error_handlers.o\
	messages.o commands.o handlers.o net_utils.o

# common dependency files
CD = ${INC}/common_headers.h ${INC}/error_handlers.h ${INC}/net_utils.h\
     ${INC}/robust_io.h ${INC}/connections.h ${INC}/messages.h ${INC}/sha1.h\
     ${INC}/bencode.h ${INC}/ubtorrent.h ${INC}/color.h 

# Programs 
PROGS = ubtorrent

all: ${PROGS}

ubtorrent: ubtorrent.c ${CD} ${COBJS}
	${CC} -I./${INC} ${CFLAGS} ${LIBS} ubtorrent.c -o $@ ${COBJS}

sha1.o: ${CD} sha1.c
	${CC} -I./${INC} ${CFLAGS} -c sha1.c

messages.o: ${CD} messages.c
	${CC} -I./${INC} ${CFLAGS} -c messages.c

commands.o: ${CD} commands.c
	${CC} -I./${INC} ${CFLAGS} -c commands.c

connections.o: ${CD} connections.c
	${CC} -I./${INC} ${CFLAGS} -c connections.c

chatty_util_functions.o: ${CD} chatty_util_functions.c
	${CC} -I./${INC} ${CFLAGS} -c chatty_util_functions.c

net_utils.o: ${CD} net_utils.c
	${CC} -I./${INC} ${CFLAGS} -c net_utils.c

error_handlers.o: ${CD} error_handlers.c
	${CC} -I./${INC} ${CFLAGS} -c error_handlers.c

bencode.o: ${CD} bencode.c
	${CC} -I./${INC} ${CFLAGS} -c bencode.c

color.o: ${CD} color.c
	${CC} -I./${INC} ${CFLAGS} -c color.c

robust_io.o: ${CD} robust_io.c
	${CC} -I./${INC} ${CFLAGS} -c robust_io.c

clean:
	rm -rf ${PROGS} ${CLEAN}

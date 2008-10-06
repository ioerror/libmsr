# This currently builds a user space program and not a useful library

CFLAGS=	-Wall -g -ansi -pedantic

LIB=	libmsr.a
LIBSRCS=	libmsr.c serialio.c msr206.c
LIBOBJS=	libmsr.o serialio.o msr206.o

UTIL=	msr
UTILSRCS=	msr.c
UTILOBJS=	msr.o

all:	$(LIB) $(UTIL)

$(LIB): $(LIBOBJS)
	ar rcs $(LIB) $(LIBOBJS)

$(UTIL): $(UTILOBJS)
	$(CC) -o $(UTIL) $(UTILOBJS) -L. -lmsr

install:
	install -m655 $(UTIL) $(DESTDIR)/usr/bin/$(UTIL)
	install -m655 $(LIB) $(DESTDIR)/usr/$(LIB)

clean:
	rm -rf *.o *~ $(LIB) $(UTIL)

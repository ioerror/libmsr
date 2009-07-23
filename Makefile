# This currently builds a user space program and not a useful library

CFLAGS=	-Wall -g -ansi -pedantic
LDFLAGS= -L. -lmsr

LIB=	libmsr.a
LIBSRCS=	libmsr.c serialio.c msr206.c makstripe.c
LIBOBJS=	$(LIBSRCS:.c=.o)

SUBDIRS=utils

all:	$(LIB)
	for subdir in $(SUBDIRS); do \
	  (cd $$subdir && $(MAKE) all); \
	done

$(LIB): $(LIBOBJS)
	ar rcs $(LIB) $(LIBOBJS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

install: 
	install -m644 -D $(LIB) $(DESTDIR)/usr/$(LIB)
	for subdir in $(SUBDIRS); do \
	  (cd $$subdir && $(MAKE) install); \
	done

clean:
	rm -rf *.o *~ $(LIB)
	for subdir in $(SUBDIRS); do \
	  (cd $$subdir && $(MAKE) clean); \
	done

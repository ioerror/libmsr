# This currently builds a user space program and not a useful library

CFLAGS=	-Wall -g -ansi -pedantic
LDFLAGS= -L. -lmsr

LIB=	libmsr.a
LIBSRCS=	libmsr.c serialio.c msr206.c
LIBOBJS=	$(LIBSRCS:.c=.o)

UTIL=	msr
UTILSRCS=	msr.c
UTILOBJS=	$(UTILSRCS:.c=.o)

SUBDIRS=utils

all:	$(LIB) $(UTIL)
	for subdir in $(SUBDIRS); do \
	  (cd $$subdir && $(MAKE) all); \
	done

$(LIB): $(LIBOBJS)
	ar rcs $(LIB) $(LIBOBJS)

$(UTIL): $(UTILOBJS)
	$(CC) -o $(UTIL) $(UTILOBJS) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

install: 
	install -m655 -D $(UTIL) $(DESTDIR)/usr/bin/$(UTIL)
	install -m655 -D $(LIB) $(DESTDIR)/usr/$(LIB)
	for subdir in $(SUBDIRS); do \
	  (cd $$subdir && $(MAKE) install); \
	done

clean:
	rm -rf *.o *~ $(LIB) $(UTIL)
	for subdir in $(SUBDIRS); do \
	  (cd $$subdir && $(MAKE) clean); \
	done

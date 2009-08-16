# This currently builds a user space program and not a useful library

CFLAGS=	-Wall -g -ansi -pedantic
LDFLAGS= -L. -lmsr

LIB=	libmsr.a
LIBSRCS=	libmsr.c serialio.c msr206.c makstripe.c
LIBOBJS=	$(LIBSRCS:.c=.o)

DAB=	dab
DABSRCS=	dab.c
DABOBJS=	$(DABSRCS:.c=.o)

DMSB=	dmsb
DMSBSRCS=	dmsb.c
DMSBOBJS=	$(DMSBSRCS:.c=.o)

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

AUDIOLDFLAGS=-lsndfile

$(DAB): $(DABOBJS)
	$(CC) -o $(DAB) $(DABOBJS) $(AUDIOLDFLAGS)
$(DMSB): $(DMSBOBJS)
	$(CC) -o $(DMSB) $(DMSBOBJS) $(AUDIOLDFLAGS)

audio: $(DAB) $(DMSB)

clean:
	rm -rf *.o *~ $(LIB) $(DAB) $(DMSB)
	for subdir in $(SUBDIRS); do \
	  (cd $$subdir && $(MAKE) clean); \
	done

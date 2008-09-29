# This currently builds a user space program and not a useful library

CFLAGS=	-Wall -g -ansi -pedantic
TARGET=	libmsr
SRCS=	msr.c serialio.c
OBJS=	msr.o serialio.o

$(TARGET): $(OBJS)
	ar rcs $(TARGET) $(OBJS)

install:
	install -m655 msr $(DESTDIR)/usr/bin/msr

clean:
	rm -rf *.o *~ $(TARGET)

# This currently builds a user space program and not a useful library

CFLAGS= -Wall
TARGET=msr
OBJECTS=msr.o

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^

install:
	install -m655 msr $(DESTDIR)/usr/bin/msr

clean:
	rm -rf *.o *~ $(TARGET)

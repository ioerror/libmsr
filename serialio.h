
#ifndef _SERIALIO_H_

extern int serial_open (char *, int *,  int, speed_t);
extern int serial_close (int);
extern int serial_readchar (int, uint8_t *);
extern int serial_write (int, void *, size_t);
extern int serial_read (int, void *, size_t);

#endif /* _SERIALIO_H_ */

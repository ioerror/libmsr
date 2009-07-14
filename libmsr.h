
/*
 * Track lengths when doing raw accesses can be at most 256 byte
 * in size, since the size field is only 8 bits wide. So we use this
 * as our maximum size.
 */

#define MSR_MAX_TRACK_LEN 255
#define MSR_MAX_TRACKS 3

typedef struct msr_track {
	uint8_t		msr_tk_data[MSR_MAX_TRACK_LEN];
	uint8_t		msr_tk_len;
} msr_track_t;

typedef struct msr_tracks {
	msr_track_t	msr_tracks[MSR_MAX_TRACKS];
} msr_tracks_t;

extern int msr_zeros (int);
extern int msr_commtest (int);
extern int msr_init (int);
extern int msr_fwrev (int);
extern int msr_model (int);
extern int msr_sensor_test (int);
extern int msr_ram_test (int);
extern int msr_set_hi_co (int);
extern int msr_set_lo_co (int);
extern int msr_iso_read (int, msr_tracks_t *);
extern int msr_iso_write (int, msr_tracks_t *);
extern int msr_raw_read (int, msr_tracks_t *);
extern int msr_raw_write (int, msr_tracks_t *);
extern int msr_erase (int, uint8_t);
extern int msr_flash_led (int, uint8_t);
extern int msr_set_bpi (int, uint8_t);
extern int msr_set_bpc (int, uint8_t, uint8_t, uint8_t);

extern int msr_dumpbits (uint8_t *, int);
extern int msr_getbit (uint8_t *, uint8_t, int);
extern int msr_setbit (uint8_t *, uint8_t, int, int);
extern int msr_decode (uint8_t *, uint8_t, uint8_t *, uint8_t *, int);

extern int msr_reverse_tracks (msr_tracks_t *);
extern int msr_reverse_track (int, msr_tracks_t *);
extern const unsigned char msr_reverse_byte (const unsigned char);

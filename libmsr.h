
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

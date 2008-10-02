
/*
 * Track lengths when doing raw accesses can be at most 256 byte
 * in size, since the size field is only 8 bits wide. So we use this
 * as our maximum size.
 */

#define MSR_MAX_TRACK_LEN 255

typedef struct msr_tracks {
	uint8_t		msr_tk1_data[MSR_MAX_TRACK_LEN];
	uint8_t		msr_tk1_len;
	uint8_t		msr_tk2_data[MSR_MAX_TRACK_LEN];
	uint8_t		msr_tk2_len;
	uint8_t		msr_tk3_data[MSR_MAX_TRACK_LEN];
	uint8_t		msr_tk3_len;
} msr_tracks_t;

/*from dab.c   */

#define DEVICE        "/dev/dsp" /* default sound card device */
#define SAMPLE_RATE   192000     /* default sample rate (hz) */
#define SILENCE_THRES 5000       /* initial silence threshold */

/* #define DISABLE_VC */

#define AUTO_THRES    30    /* pct of highest value to set silence_thres to */
#define BUF_SIZE      1024  /* buffer size */
#define END_LENGTH    200   /* msec of silence to determine end of sample */
#define FREQ_THRES    60    /* frequency threshold (pct) */
#define MAX_TERM      60    /* sec before termination of msr_print_max_level() */



/* from dmsb.c   */

extern short int *sample = NULL;
extern int sample_size = 0;

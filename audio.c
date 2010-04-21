
#include <fcntl.h>
#include <getopt.h>
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "audio.h"
#include "libmsr.h"


/********** string functions **********/

/* returns a pointer to the reversed string
   [string]        string to reverse
   returns         newly allocated reversed string */
char *reverse_string(char *string)
{
  char *rstring;
  int i, string_len;

  string_len = strlen(string); /* record string length */

  /* allocate memory for rstring */
  rstring = xmalloc(string_len + 1);

  for (i = 0; i < string_len; i++) /* reverse string and store in rstring */
    rstring[i] = string[string_len - i - 1];

  rstring[string_len] = '\0'; /* terminate rstring */

  return rstring; /* return rstring */
}

/********** end string functions **********/





/********** parsing functions **********/

/* parse ABA format raw bits and return a pointer to the decoded string
   [bitstring]     string to decode
   returns         decoded string */
char *parse_ABA(char *bitstring)
{
  char *decoded_string, *lrc_start, *start_decode, *string;
  char lrc[] = {1, 1, 0, 1, 0}; /* initial condition is LRC of the start
                                   sentinel */
  int asciichr, charcnt = 0, i, j;

  /* make a copy of bitstring and store it in string */
  string = xstrdup(bitstring);

  /* look for start sentinel */
  if ((start_decode = strstr(string, "11010")) == NULL) {
    free(string); /* free string memory */
    return NULL; /* could not find start sentinel */
  }

  /* set start_decode to first bit (start of first byte) after start
     sentinel */
  start_decode += 5;

  /* look for end sentinel */
  if ((lrc_start = strstr(string, "11111")) == NULL) {
    free(string); /* free string memory */
    return NULL; /* could not find end sentinel */
  }
  /* must be a multiple of 5 */
  while ((strlen(start_decode) - strlen(lrc_start)) % 5) /* search again */
    if ((lrc_start = strstr(++lrc_start, "11111")) == NULL) {
      free(string); /* free string memory */
      return NULL; /* could not find end sentinel */
    }

  lrc_start[0] = '\0'; /* terminate start_decode at end sentinel */

  lrc_start += 5; /* set the pointer to the LRC */
  if (lrc_start[5] != '\0') /* terminate LRC if not already */
    lrc_start[5] = '\0';

  /* allocate memory for decoded_string */
  decoded_string = xmalloc((strlen(start_decode) / 5) + 3);

  decoded_string[charcnt++] = ';'; /* add start sentinel */

  /* decode each set of bits, check parity, check LRC, and add to
     decoded_string */
  while (strlen(start_decode)) {

    for (i = 0, j = 0; i < 4; i++) /* check parity */
      if (start_decode[i] == '1')
        j++;
    if (((j % 2) && start_decode[4] == '1') ||
        (!(j % 2) && start_decode[4] == '0')) {
      free(string); /* free string memory */
      free(decoded_string); /* free decoded_string memory */
      return NULL; /* failed parity check */
    }

    asciichr = 48; /* generate ascii value from bits */
    asciichr += start_decode[0] == '1' ? 1 : 0;
    asciichr += start_decode[1] == '1' ? 2 : 0;
    asciichr += start_decode[2] == '1' ? 4 : 0;
    asciichr += start_decode[3] == '1' ? 8 : 0;

    decoded_string[charcnt++] = asciichr; /* add character to decoded_string */

    for (i = 0; i < 4; i++) /* calculate LRC */
      lrc[i] = lrc[i] ^ (start_decode[i] == '1') ? 1 : 0;

    start_decode += 5; /* increment start_decode to next byte */
  }

  decoded_string[charcnt++] = '?'; /* add end sentinel */
  decoded_string[charcnt] = '\0'; /* terminate decoded_string */

  for (i = 0; i < 4; i++) /* calculate CRC of end sentinel */
    lrc[i] = lrc[i] ^ 1;

  for (i = 0, j = 0; i < 4; i++) /* set LRC parity bit */
    if (lrc[i])
      j++;
  if (!(j % 2))
    lrc[4] = 1;
  else
    lrc[4] = 0;

  for (i = 0; i < 5; i++) /* check CRC */
    if ((lrc[i] && lrc_start[i] == '0') ||
        (!lrc[i] && lrc_start[i] == '1')) {
      free(string); /* free string memory */
      free(decoded_string); /* free decoded_string memory */
      return NULL; /* failed CRC check */
    }

  free(string); /* free string memory */
  return decoded_string;
}


/* parse IATA format raw bits and return a pointer to the decoded string
   [bitstring]     string to decode
   returns         decoded string */
char *parse_IATA(char *bitstring)
{
  char *decoded_string, *lrc_start, *start_decode, *string;
  char lrc[] = {1, 0, 1, 0, 0, 0, 1}; /* initial condition is LRC of the start
                                         sentinel */
  int asciichr, charcnt = 0, i, j;

  /* make a copy of bitstring and store it in string */
  string = xstrdup(bitstring);

  /* look for start sentinel */
  if ((start_decode = strstr(string, "1010001")) == NULL) {
    free(string); /* free string memory */
    return NULL; /* could not find start sentinel */
  }

  /* set start_decode to first bit (start of first byte) after start
     sentinel */
  start_decode += 7;

  /* look for end sentinel */
  if ((lrc_start = strstr(string, "1111100")) == NULL) {
    free(string); /* free string memory */
    return NULL; /* could not find end sentinel */
  }
  /* must be a multiple of 7 */
  while ((strlen(start_decode) - strlen(lrc_start)) % 7)
    /* search again */
    if ((lrc_start = strstr(++lrc_start, "1111100")) == NULL) {
      free(string); /* free string memory */
      return NULL; /* could not find end sentinel */
    }

  lrc_start[0] = '\0'; /* terminate start_decode at end sentinel */

  lrc_start += 7; /* set the pointer to the LRC */
  if (lrc_start[7] != '\0') /* terminate LRC if not already */
    lrc_start[7] = '\0';

  /* allocate memory for decoded_string */
  decoded_string = xmalloc((strlen(start_decode) / 7) + 3);

  decoded_string[charcnt++] = '%'; /* add start sentinel */

  /* decode each set of bits, check parity, check LRC, and add to
     decoded_string */
  while (strlen(start_decode)) {

    for (i = 0, j = 0; i < 6; i++) /* check parity */
      if (start_decode[i] == '1')
        j++;
    if (((j % 2) && start_decode[6] == '1') ||
        (!(j % 2) && start_decode[6] == '0')) {
      free(string); /* free string memory */
      free(decoded_string); /* free decoded_string memory */
      return NULL; /* failed parity check */
    }

    asciichr = 32; /* generate ascii value from bits */
    asciichr += start_decode[0] == '1' ? 1  : 0;
    asciichr += start_decode[1] == '1' ? 2  : 0;
    asciichr += start_decode[2] == '1' ? 4  : 0;
    asciichr += start_decode[3] == '1' ? 8  : 0;
    asciichr += start_decode[4] == '1' ? 16 : 0;
    asciichr += start_decode[5] == '1' ? 32 : 0;

    decoded_string[charcnt++] = asciichr; /* add character to decoded_string */

    for (i = 0; i < 6; i++) /* calculate LRC */
      lrc[i] = lrc[i] ^ (start_decode[i] == '1') ? 1 : 0;

    start_decode += 7; /* increment start_decode to next byte */
  }

  decoded_string[charcnt++] = '?'; /* add end sentinel */
  decoded_string[charcnt] = '\0'; /* terminate decoded_string */

  for (i = 0; i < 5; i++) /* calculate CRC of end sentinel */
    lrc[i] = lrc[i] ^ 1;
  lrc[5] = lrc[5] ^ 0;

  for (i = 0, j = 0; i < 6; i++) /* set LRC parity bit */
    if (lrc[i])
      j++;
  if (!(j % 2))
    lrc[6] = 1;
  else
    lrc[6] = 0;

  for (i = 0; i < 7; i++) /* check CRC */
    if ((lrc[i] && lrc_start[i] == '0') ||
        (!lrc[i] && lrc_start[i] == '1')) {
      free(string); /* free string memory */
      free(decoded_string); /* free decoded_string memory */
      return NULL; /* failed CRC check */
    }

  free(string); /* free string memory */
  return decoded_string;

}

/********** end parsing functions **********/




/********** function wrappers **********/

/* allocate memory with out of memory checking
   [size]          allocate size bytes
   returns         pointer to allocated memory */
void *xmalloc(size_t size)
{
  void *ptr;

  ptr = malloc(size);
  if (ptr == NULL) {
    fprintf(stderr, "Out of memory.\n");
    exit(EXIT_FAILURE);
  }

  return ptr;
}



/* reallocate memory with out of memory checking
   [ptr]           memory to reallocate
   [size]          allocate size bytes
   returns         pointer to reallocated memory */
void *xrealloc(void *ptr, size_t size)
{
  void *nptr;

  nptr = realloc(ptr, size);
  if (nptr == NULL) {
    fprintf(stderr, "Out of memory.\n");
    exit(EXIT_FAILURE);
  }

  return nptr;
}


/* copy a string with out of memory checking
   [string]         string to copy
   returns          newly allocated copy of string */
char *xstrdup(char *string)
{
  char *ptr;

  ptr = xmalloc(strlen(string) + 1);
  strcpy(ptr, string);

  return ptr;
}


/* read with error checking
   [fd]            file descriptor to read from
   [buf]           buffer
   [count]         bytes to read
   returns         bytes read */
ssize_t xread(int fd, void *buf, size_t count)
{
  int retval;

  retval = read(fd, buf, count);
  if (retval == -1) {
    perror("read()");
    exit(EXIT_FAILURE);
  }

  return retval;
}

/********** end function wrappers **********/




/********** dsp functions **********/

/* sets the device parameters
   [fd]            file descriptor to set ioctls on
   [verbose]       prints verbose messages if true
   returns         sample rate */
int dsp_init(int fd, int verbose)
{
  int ch, fmt, sr;

  if (verbose)
    fprintf(stderr, "*** Setting audio device parameters:\n");

  /* set audio format */
  if (verbose)
    fprintf(stderr, "    Format: AFMT_S16_LE\n");
  fmt = AFMT_S16_LE;
  if (ioctl(fd, SNDCTL_DSP_SETFMT, &fmt) == -1) {
    perror("SNDCTL_DSP_SETFMT");
    exit(EXIT_FAILURE);
  }
  if (fmt != AFMT_S16_LE) {
    fprintf(stderr, "*** Error: Device does not support AFMT_S16_LE\n");
    exit(EXIT_FAILURE);
  }

  /* set audio channels */
  if (verbose)
    fprintf(stderr, "    Channels: 1\n");
  ch = 0;
  if (ioctl(fd, SNDCTL_DSP_STEREO, &ch) == -1) {
    perror("SNDCTL_DSP_STEREO");
    exit(EXIT_FAILURE);
  }
  if (ch != 0) {
    fprintf(stderr, "*** Error: Device does not support monaural recording\n");
    exit(EXIT_FAILURE);
  }

  /* set sample rate */
  if (verbose)
    fprintf(stderr, "    Sample rate: %d\n", SAMPLE_RATE);
  sr = SAMPLE_RATE;
  if (ioctl(fd, SNDCTL_DSP_SPEED, &sr) == -1) {
    perror("SNDCTL_DSP_SPEED");
    exit(EXIT_FAILURE);
  }
  if (sr != SAMPLE_RATE)
    fprintf(stderr, "*** Warning: Highest supported sample rate is %d\n", sr);

  return sr;
}


/* prints the maximum dsp level to aid in setting the silence threshold
   [fd]            file descriptor to read from
   [sample_rate]   sample rate of device */
void print_max_level(int fd, int sample_rate)
{
  int i;
  short int buf, last = 0;

  printf("Terminating after %d seconds...\n", MAX_TERM);

  for (i = 0; i < sample_rate * MAX_TERM; i++) {

    /* read from fd */
    xread(fd, &buf, sizeof (short int));

    /* take absolute value */
    if (buf < 0)
      buf = -buf;

    /* print if highest level */
    if (buf > last) {
      printf("Maximum level: %d\r", buf);
      fflush(stdout);
      last = buf;
    }
  }

  printf("\n");
}


/* finds the maximum value in sample
   ** global **
   [sample]        sample
   [sample_size]   number of frames in sample */
short int evaluate_max(void)
{
  int i;
  short int max = 0;

  for (i = 0; i < sample_size; i++) {
    if (sample[i] > max)
       max = sample[i];
  }

  return max;
}


/* pauses until the dsp level is above the silence threshold
   [fd]            file descriptor to read from
   [silence_thres] silence threshold */
void silence_pause(int fd, int silence_thres)
{
  short int buf = 0;

  /* loop while silent */
  while (buf < silence_thres) {

    /* read from fd */
    xread(fd, &buf, sizeof (short int));

    /* absolute value */
    if (buf < 0)
      buf = -buf;
  }
}


/* gets a sample, terminating when the input goes below the silence threshold
   [fd]            file descriptor to read from
   [sample_rate]   sample rate of device
   [silence_thres] silence threshold
   ** global **
   [sample]        sample
   [sample_size]   number of frames in sample */
void get_dsp(int fd, int sample_rate, int silence_thres)
{
  int count = 0, eos = 0, i;
  short buf;

  sample_size = 0;

  /* wait for sample */
  silence_pause(fd, silence_thres);

  while (!eos) {
    /* fill buffer */
    sample = xrealloc(sample, sizeof (short int) * (BUF_SIZE * (count + 1)));
    for (i = 0; i < BUF_SIZE; i++) {
      xread(fd, &buf, sizeof (short int));
      sample[i + (count * BUF_SIZE)] = buf;
    }
    count++;
    sample_size = count * BUF_SIZE;

    /* check for silence */
    eos = 1;
    if (sample_size > (sample_rate * END_LENGTH) / 1000) {
      for (i = 0; i < (sample_rate * END_LENGTH) / 1000; i++)  {
        buf = sample[(count * BUF_SIZE) - i - 1];
        if (buf < 0)
          buf = -buf;
        if (buf > silence_thres)
          eos = 0;
      }
    } else
      eos = 0;
  }
}

/********** end dsp functions **********/





/********** begin sndfile functions **********/

/* open the file
   [fd]          file to open
   [verbose]     verbosity flag
   ** global **
   [sample_size] number of frames in the file */
SNDFILE *sndfile_init(int fd, int verbose)
{
  SNDFILE *sndfile;
  SF_INFO sfinfo;

  /* clear sfinfo structure */
  memset(&sfinfo, 0, sizeof(sfinfo));

  /* set sndfile from file descriptor */
  sndfile = sf_open_fd(fd, SFM_READ, &sfinfo, 0);
  if (sndfile == NULL) {
    fprintf(stderr, "*** Error: sf_open_fd() failed\n");
    exit(EXIT_FAILURE);
  }

  /* print some statistics */
  if (verbose) {
    fprintf(stderr, "*** Input file format:\n"
            "    Frames: %i\n"
            "    Sample Rate: %i\n"
            "    Channels: %i\n"
            "    Format: 0x%08x\n"
            "    Sections: %i\n"
            "    Seekable: %i\n",
            (int)sfinfo.frames, sfinfo.samplerate, sfinfo.channels,
            sfinfo.format, sfinfo.sections, sfinfo.seekable);
  }

  /* ensure that the file is monaural */
  if (sfinfo.channels != 1) {
    fprintf(stderr, "*** Error: Only monaural files are supported\n");
    exit(EXIT_FAILURE);
  }

  /* set sample size */
  sample_size = sfinfo.frames;

  return sndfile;
}


/* read in data from libsndfile
   [sndfile]     SNDFILE pointer from sf_open() or sf_open_fd()
   ** global **
   [sample]      sample
   [sample_size] number of frames in sample */
void get_sndfile(SNDFILE *sndfile)
{
  sf_count_t count;

  /* allocate memory for sample */
  sample = xmalloc(sizeof(short int) * sample_size);

  /* read in sample */
  count = sf_read_short(sndfile, sample, sample_size);
  if (count != sample_size) {
    fprintf(stderr, "*** Warning: expected %i frames, read %i.\n",
            sample_size, (int)count);
    sample_size = count;
  }
}

/********** end sndfile functions **********/





/* decodes aiken biphase and prints binary
   [freq_thres]    frequency threshold
   ** global **
   [sample]        sample
   [sample_size]   number of frames in sample */
void decode_aiken_biphase(int freq_thres, int silence_thres)
{
  int i = 0, peak = 0, ppeak = 0;
  int *peaks = NULL, peaks_size = 0;
  int zerobl;

  /* absolute value */
  for (i = 0; i < sample_size; i++)
    if (sample[i] < 0)
      sample[i] = -sample[i];

  /* store peak differences */
  i = 0;
  while (i < sample_size) {
    /* old peak value */
    ppeak = peak;
    /* find peaks */
    while (i < sample_size && sample[i] <= silence_thres)
      i++;
    peak = 0;
    while (i < sample_size && sample[i] > silence_thres) {
      if (sample[i] > sample[peak])
        peak = i;
      i++;
    }
    if (peak - ppeak > 0) {
      peaks = xrealloc(peaks, sizeof(int) * (peaks_size + 1));
      peaks[peaks_size] = peak - ppeak;
      peaks_size++;
    }
  }

  /* decode aiken biphase allowing for
     frequency deviation based on freq_thres */
  /* ignore first two peaks and last peak */
  if (peaks_size < 2) {
    fprintf(stderr, "*** Error: No data detected\n");
    exit(EXIT_FAILURE);
  }
  zerobl = peaks[2];
  for (i = 2; i < peaks_size - 1; i++) {
    if (peaks[i] < ((zerobl / 2) + (freq_thres * (zerobl / 2) / 100)) &&
        peaks[i] > ((zerobl / 2) - (freq_thres * (zerobl / 2) / 100))) {
      if (peaks[i + 1] < ((zerobl / 2) + (freq_thres * (zerobl / 2) / 100)) &&
          peaks[i + 1] > ((zerobl / 2) - (freq_thres * (zerobl / 2) / 100))) {
        printf("1");
        zerobl = peaks[i] * 2;
        i++;
      }
    } else if (peaks[i] < (zerobl + (freq_thres * zerobl / 100)) &&
               peaks[i] > (zerobl - (freq_thres * zerobl / 100))) {
      printf("0");
#ifndef DISABLE_VC
      zerobl = peaks[i];
#endif
    }
  }
  printf("\n");

}

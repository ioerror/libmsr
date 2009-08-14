/* dab.c - Decode Aiken Biphase
   
   Copyright (c) 2004-2005 Joseph Battaglia <sephail@sephail.net>
   
   Code contributions / patches:
     Mike Castleman <mlc@2600.com>
     Ed Wandasiewicz <wanded@breathemail.net>
   
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.
   
   Changelog:
     0.1 (Sep 2004):
          'audiomag' released
     0.2 (Oct 2004):
          2600 MetroCard decoding project started
          changed name from 'audiomag' to 'dab'
          now requires only one "clocking" bit
          optimized for reading non-standard cards (eg. MetroCards)
     0.3 (Nov 2004):
          improved decoding algorithm
          added max_level functionality
     0.4 (Dec 2004):
          fixed bug when calculating threshold from percentage
     0.5 (Dec 2004):
          improved decoding algorithm
          improved automatic threshold detection
          added support for reading from a file with libsndfile (Mike C.)
     0.6 (Jan 2005):
          fixed broken flags
          improved libsndfile use
     0.7 (Aug 2005):
          fixed potential segmentation fault (Ed W.)
   
   Compiling:
     cc dab.c -o dab -lsndfile
*/


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

/*** defaults ***/
#define DEVICE        "/dev/dsp" /* default sound card device */
#define SAMPLE_RATE   192000     /* default sample rate (hz) */
#define SILENCE_THRES 5000       /* initial silence threshold */
/*** end defaults ***/

/* #define DISABLE_VC */

#define AUTO_THRES    30    /* pct of highest value to set silence_thres to */
#define BUF_SIZE      1024  /* buffer size */
#define END_LENGTH    200   /* msec of silence to determine end of sample */
#define FREQ_THRES    60    /* frequency threshold (pct) */
#define MAX_TERM      60    /* sec before termination of print_max_level() */
#define VERSION       "0.7" /* version */


short int *sample = NULL;
int sample_size = 0;





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





/********** version functions **********/

/* prints version
   [stream]        output stream */
void print_version(FILE *stream)
{
  fprintf(stream, "dab - Decode Aiken Biphase\n");
  fprintf(stream, "Version %s\n", VERSION);
  fprintf(stream, "Copyright (c) 2004-2005 ");
  fprintf(stream, "Joseph Battaglia <sephail@sephail.net>\n");
}


/* prints version and help 
   [stream]        output stream
   [exec]          string containing the name of the program executable */
void print_help(FILE *stream, char *exec)
{
  print_version(stream);
  fprintf(stream, "\nUsage: %s [OPTIONS]\n\n", exec);
  fprintf(stream, "  -a,  --auto-thres   Set auto-thres percentage\n");
  fprintf(stream, "                      (default: %d)\n", AUTO_THRES);
  fprintf(stream, "  -d,  --device       Device to read audio data from\n");
  fprintf(stream, "                      (default: %s)\n", DEVICE);
  fprintf(stream, "  -f,  --file         File to read audio data from\n");
  fprintf(stream, "                      (use instead of -d)\n");
  fprintf(stream, "  -h,  --help         Print help information\n");
  fprintf(stream, "  -m,  --max-level    Shows the maximum level\n");
  fprintf(stream, "                      (use to determine threshold)\n");
  fprintf(stream, "  -s,  --silent       No verbose messages\n");
  fprintf(stream, "  -t,  --threshold    Set silence threshold\n");
  fprintf(stream, "                      (default: automatic detect)\n");
  fprintf(stream, "  -v,  --version      Print version information\n");
}

/********** end version functions **********/





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





/* main */
int main(int argc, char *argv[])
{
  int fd;
  SNDFILE *sndfile = NULL;
  
  /* configuration variables */
  char *filename = NULL;
  int auto_thres = AUTO_THRES, max_level = 0, use_sndfile = 0, verbose = 1;
  int sample_rate = SAMPLE_RATE, silence_thres = SILENCE_THRES;
  
  /* getopt variables */
  int ch, option_index;
  static struct option long_options[] = {
    {"auto-thres",   0, 0, 'a'},
    {"device",       1, 0, 'd'},
    {"file",         1, 0, 'f'},
    {"help",         0, 0, 'h'},
    {"max-level",    0, 0, 'm'},
    {"silent",       0, 0, 's'},
    {"threshold",    1, 0, 't'},
    {"version",      0, 0, 'v'},
    { 0,             0, 0,  0 }
  };
  
  /* process command line arguments */
  while (1) {
    
    ch = getopt_long(argc, argv, "a:d:f:hmst:v", long_options, &option_index);
    
    if (ch == -1)
      break;
    
    switch (ch) {
      /* auto-thres */
      case 'a':
        auto_thres = atoi(optarg);
        break;
      /* device */
      case 'd':
        filename = xstrdup(optarg);
        break;
      /* file */
      case 'f':
        filename = xstrdup(optarg);
        use_sndfile = 1;
        break;
      /* help */
      case 'h':
        print_help(stdout, argv[0]);
        exit(EXIT_SUCCESS);
        break;
      /* max-level */
      case 'm':
        max_level = 1;
        break;
      /* silent */
      case 's':
        verbose = 0;
        break;
      /* threshold */
      case 't':
        auto_thres = 0;
        silence_thres = atoi(optarg);
        break;
      /* version */
      case 'v':
        print_version(stdout);
        exit(EXIT_SUCCESS);
        break;
      /* default */
      default:
        print_help(stderr, argv[0]);
        exit(EXIT_FAILURE);
        break;
    }
  }
  
  /* print version */
  if (verbose) {
    print_version(stderr);
    fprintf(stderr, "\n");
  }
  
  /* check for incorrect use of command-line arguments */
  if (use_sndfile && max_level) {
    fprintf(stderr, "*** Error: -f and -m switches do not mix!\n");
    exit(EXIT_FAILURE);
  }
  
  /* set default if no device is specified */
  if (filename == NULL)
    filename = xstrdup(DEVICE);
  
  /* open device for reading */
  if (verbose)
    fprintf(stderr, "*** Opening %s\n", filename);
  fd = open(filename, O_RDONLY);
  if (fd == -1) {
    perror("open()");
    exit(EXIT_FAILURE);
  }
  
  /* open sndfile or set device parameters */
  if (use_sndfile)
    sndfile = sndfile_init(fd, verbose);
  else
    sample_rate = dsp_init(fd, verbose);
  
  /* show user maximum dsp level */
  if (max_level) {
    print_max_level(fd, sample_rate);
    exit(EXIT_SUCCESS);
  }
  
  /* silence_thres sanity check */
  if (!silence_thres) {
    fprintf(stderr, "*** Error: Invalid silence threshold\n");
    exit(EXIT_FAILURE);
  }
  
  /* read sample */
  if (use_sndfile)
    get_sndfile(sndfile);
  else {
    if (verbose)
      fprintf(stderr, "*** Waiting for sample...\n");
    get_dsp(fd, sample_rate, silence_thres);
  }
  
  /* automatically set threshold */
  if (auto_thres)
    silence_thres = auto_thres * evaluate_max() / 100;
  
  /* print silence threshold */
  if (verbose)
    fprintf(stderr, "*** Silence threshold: %d (%d%% of max)\n",
            silence_thres, auto_thres);
  
  /* decode aiken biphase */
  decode_aiken_biphase(FREQ_THRES, silence_thres);
  
  /* close file */
  close(fd);
  
  /* free memory */
  free(sample);
  
  exit(EXIT_SUCCESS);
  
  return 0;
}

/* end dab.c */

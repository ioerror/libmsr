

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

#include "libmsr.h"
#include "libmsr.h"
#include "serialio.h"

short int *sample = NULL;
int sample_size = 0;




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

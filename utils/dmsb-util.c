

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "libmsr.h"
#include "libmsr.h"
#include "serialio.h"


int main(int argc, char *argv[])
{
  char buf[BUF_SIZE], *rbuf, *decoded_data;
  int verbose = 0;
  int ch, option_index;

  static struct option long_options[] = {
    {"verbose", 0, 0, 'V'},
    {"help"   , 0, 0, 'h'},
    {"version", 0, 0, 'v'},
    { 0       , 0, 0,  0 }
  };

  while ((ch = getopt_long(argc, argv, "Vhv", long_options, &option_index))
         != -1) {
    switch (ch)
    {
      case 'V': /* verbose */
        verbose = 1;
        break;
      case 'h': /* help */
        print_help(stdout, argv[0]);
        exit(EXIT_SUCCESS);
        break;
      case 'v': /* version */
        print_version(stdout);
        exit(EXIT_SUCCESS);
        break;
      default: /* invalid option */
        print_help(stderr, argv[0]);
        exit(EXIT_FAILURE);
        break;
    }
  }

  if (verbose) {
    print_version(stderr);
    fprintf(stderr, "Waiting for data on stdin...\n");
  }

  fgets(buf, BUF_SIZE, stdin); /* get string from stdin */

  if (verbose) {
    fprintf(stderr, "Trying to decode using ABA...");
    fflush(stderr);
  }


  if ((decoded_data = parse_ABA(buf)) != NULL) { /* try ABA */
    if (verbose) {
      fprintf(stderr, "success\n");
      fprintf(stderr, "ABA format detected:\n");
    }
    printf("%s\n", decoded_data); /* print decoded data */
    exit(EXIT_SUCCESS);
  }

  if (verbose) {
    fprintf(stderr, "reversing bits...");
    fflush(stderr);
  }

  rbuf = reverse_string(buf); /* reverse string and try again */

  if ((decoded_data = parse_ABA(rbuf)) != NULL) { /* try ABA */
    if (verbose) {
      fprintf(stderr, "success\n");
      fprintf(stderr, "ABA format detected (bits reversed):\n");
    }
    printf("%s\n", decoded_data);
    exit(EXIT_SUCCESS);
  }

  if (verbose)
    fprintf(stderr, "failed\n");

  if (verbose) {
    fprintf(stderr, "Trying to decode using IATA...");
    fflush(stderr);
  }


  if ((decoded_data = parse_IATA(buf)) != NULL) { /* try IATA */
    if (verbose) {
      fprintf(stderr, "success\n");
      fprintf(stderr, "IATA format detected:\n");
    }
    printf("%s\n", decoded_data); /* print decoded data */
    exit(EXIT_SUCCESS);
  }

  if (verbose) {
    fprintf(stderr, "reversing bits...");
    fflush(stderr);
  }

  if ((decoded_data = parse_IATA(rbuf)) != NULL) { /* try IATA with reverse */
    if (verbose) {
      fprintf(stderr, "success\n");
      fprintf(stderr, "IATA format detected (bits reversed):\n");
    }
    printf("%s\n", decoded_data);
    exit(EXIT_SUCCESS);
  }

  if (verbose)
    fprintf(stderr, "failed\n");

  printf("Detection failed\n");

  exit(EXIT_FAILURE);

  return 0;
}

/* end dmsb.c */

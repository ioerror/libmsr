/* dmsb.c - Decodes (standard) Magnetic Stripe Binary
   
   Copyright (c) 2004 Joseph Battaglia <sephail@sephail.net>
   
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
   
   Compiling:
     cc dmsb.c -o dmsb
*/


#define BUF_SIZE 2048
#define VERSION "0.1"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>





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

/********** end function wrappers **********/





/********** version functions **********/

/* print version information
   [stream]        output stream */
void print_version(FILE *stream)
{
  fprintf(stream, "dmsb - Decode (standard) Magnetic Stripe Binary\n");
  fprintf(stream, "Version %s\n", VERSION);
  fprintf(stream, "Copyright (c) 2004 Joseph Battaglia <sephail@sephail.net>\n");
}


/* print help information
   [stream]        output stream
   [exec]          string containing the name of the program executable */
void print_help(FILE *stream, char *exec)
{
  print_version(stream);
  fprintf(stream, "\nUsage: %s [OPTIONS]\n", exec);
  fprintf(stream, "\n");
  fprintf(stream, "  -V,  --verbose      Verbose messages\n");
  fprintf(stream, "\n");
  fprintf(stream, "  -h,  --help         Print help information\n");
  fprintf(stream, "  -v,  --version      Print version information\n");
  fprintf(stream, "\n");
  fprintf(stream, "dmsb will wait on stdin for raw magnetic stripe ");
  fprintf(stream, "data (string of 0s and 1s\n");
  fprintf(stream, "followed by a newline) and print the decoded data to ");
  fprintf(stream, "stdout.\n");
}

/********** end version functions **********/





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

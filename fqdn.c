/**
 * Description: A simple tool for validating domain names.
 * Author: Andreas Bank, andreas.mikael.bank@gmail.com
 * Date: 2018-02-09
 * License: This source code is Public Domain.
 */

#include <arpa/inet.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BOOL int
#define FALSE (0)
#define TRUE !FALSE
#define BYTE unsigned char

static BOOL be_verbose = FALSE;

static
verbose_printf(const char *fmt, ...)
{
  if (!be_verbose) {
    return;
  }

  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

static void
usage(const char *program_name)
{
  printf("Usage: %s [-v] <string-to-validate>\n", program_name);
}

static int
verify_fqdn(const char *string)
{
  size_t total_length = strlen(string);

  enum {
    ERROR_INVALID_CHARS,
    ERROR_INVALID_LABEL_START,
    ERROR_INVALID_LABEL_END,
    ERROR_LONG_STRING,
    ERROR_LONG_LABEL
  } error_code;

  if (total_length > 255) {
    error_code = ERROR_LONG_STRING;
    goto error;
  }

  if (string && !isalnum(string[0])) {
    /* If the string starts with anything but an alphanumerical */
    error_code = ERROR_INVALID_LABEL_START;
    goto error;
  }

  const char *label_start = NULL;
  const char *label_end = string;

  while (label_end && (label_end[0] != '\0')) {

    /* Get positions of a single label */
    label_start = label_start ? label_end + 1 : string;
    if (label_start[0] == '\0') {
      error_code = ERROR_INVALID_CHARS;
      goto error;
    }
    if ((label_end = strchr(label_start, '.')) == NULL) {
      label_end = label_start + total_length;
    }

    /* Check label size and set remaining size in 'totalsize' */
    size_t label_length = label_end - label_start;
    total_length -= label_length + 1;
    if (label_length > 63) {
      error_code = ERROR_LONG_LABEL;
      goto error;
    }

    /* Check that the allowed FQDN characters
       are not at the string beginning */
    if ((label_start[0] == '-') || (label_start[label_length - 1] == '-') ||
        (label_start[0] == '.') || (label_start[label_length - 1] == '.')) {
      error_code = ERROR_INVALID_LABEL_START;
      goto error;
    }

    /* Check that all the label characters are alphanumerical or
       if it is the last label that all characters are alpha */
    size_t i;
    for (i = 0; i < label_length; i++) {
      if (((label_end[0] != '\0') && !isalnum(label_start[i])) ||
          ((label_end[0] == '\0') && !isalpha(label_start[i]))) {
        error_code = ERROR_INVALID_CHARS;
        goto error;
      }
    }
  } /* while() */

  return 0;

error:
  switch (error_code) {
  case ERROR_INVALID_CHARS:
    verbose_printf("Invalid characters found in string\n");
    break;
  case ERROR_INVALID_LABEL_START:
    verbose_printf("Invalid first character found in string label (example "
        "'subdomain.-domain'\n");
    break;
  case ERROR_INVALID_LABEL_END:
    verbose_printf("Invalid last character found in string label (example "
        "'subdomain.domain.'\n");
    break;
  case ERROR_LONG_STRING:
    verbose_printf("String is too long (> 255)\n");
    break;
  case ERROR_LONG_LABEL:
    verbose_printf("Too long label (> 63)\n");
    break;
  };

  return 1;
}

int
main (int argc, char *argv[])
{
  if ((argc < 2) || (argc > 3)) {
    printf("Error: Wrong number of arguments\n");
    usage(argv[0]);
    return 1;
  }

  if ((argc == 3)) {
    if (strcmp(argv[1], "-v") == 0) {
      be_verbose = TRUE;
    } else {
      printf("Error: Invalid first argument '%s'\n", argv[0]);
      usage(argv[0]);
      return 1;
    }
  }

  BYTE addr_buf[sizeof (struct in6_addr)];
  char *string = argv[argc - 1];

  if ((inet_pton(AF_INET, string, &addr_buf) != 1) &&
      (inet_pton(AF_INET6, string, &addr_buf) != 1) &&
      verify_fqdn(string)) {

    verbose_printf("The string is not a valid FQDN.\n");
    return 1;
  }

  verbose_printf("The string is a valid FQDN.\n");
  return 0;
}

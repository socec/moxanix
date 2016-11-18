#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#define DATABUF_LEN 128

#define TIMESTAMP_FORMAT "%Y-%m-%dT%H:%M:%S" /* ISO 8601 */
#define TIMESTAMP_LEN 20+1 /* calculated according to the timestamp format */

/* Global variables used throughout the application. */
int debug_messages;		/* if > 0 debug messages will be printed */

/* Global functions used throughout the application. */

/**
 * Converts time in seconds from Epoch to a conveniently formatted string.
 */
void time2string(time_t time, char* timestamp);

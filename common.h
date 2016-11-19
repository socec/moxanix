/* Common header file reused within the project. */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

/* ========================================================================== */

#define BUFFER_LEN 128 /* length of a data buffer */

/* ========================================================================== */

int debug_messages;	/* if > 0 debug messages will be printed */

/* ========================================================================== */

#define TIMESTAMP_FORMAT "%Y-%m-%dT%H:%M:%S" /* follow ISO 8601 format */
#define TIMESTAMP_LEN 20+1 /* size of the timestamp format above */

/**
 * Converts time in "seconds from Epoch" to a conveniently formatted string.
 */
void time2string(time_t time, char* timestamp);

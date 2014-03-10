#include "moxerver.h"


/* structure for holding telnet option name and value */
struct telnet_option_t {
	const char *name;
	char value;
};

/* supported telnet option values */
struct telnet_option_t telnet_options[] = {
	{"WILL", 251},
	{"WONT", 252},
	{"DO", 253},
	{"DONT", 254},
	{"IAC", 255},
	{"ECHO", 1},
	{"SGA", 3},
	{"LINEMODE", 34},
	{"SLE", 45}
};
#define TELNET_OPTIONS_COUNT 9 /* keep this up with the number of supported options */

/* Returns telnet option name based on the value. */
static const char* telnet_option_name(int value) {
	int i;
	for (i = 0; i < TELNET_OPTIONS_COUNT; i ++)
		if (telnet_options[i].value == value)
			return telnet_options[i].name;
	return '\0';
}

/* Returns telnet option value based on the name. */
static char telnet_option_value(const char* name) {
	int i;
	for (i = 0; i < TELNET_OPTIONS_COUNT; i ++)
		if (!strcmp(telnet_options[i].name, name))
			return telnet_options[i].value;
	return 0;
}

/* Sends telnet option command. */
int telnet_send_command(struct client_t *client, const char* option, const char* command) {
	
	char data[3];
	
	/* pack command */
	data[0] = telnet_option_value("IAC");
	data[1] = telnet_option_value(option);
	data[2] = telnet_option_value(command);
	
	/* send command */
	if (client_write(client, data, 3) <= 0) {
		fprintf(stderr, "[%s:%d] failed sending %s %s\n", __func__, __LINE__, option, command);
		return -1;
	}
	
	fprintf(stderr, "[%s] sent %s %s\n", __func__, option, command);
	return 0;
}

/* Handles received telnet option command. */
int telnet_handle_command(char *databuf, int datalen) {
	
	if (databuf[0] == telnet_option_value("IAC")) {
		fprintf(stderr, "[%s] received %s %s\n", __func__,
				telnet_option_name(databuf[1]), telnet_option_name(databuf[2]));
	}
	
	return 0;
}

/* Tells client to go into "character" mode. */
int telnet_set_character_mode(struct client_t *client) {
	
	int err = 0;
	
	/* send predefined commands and add up their return vaules */
	err += telnet_send_command(client, "WILL", "ECHO");
	err += telnet_send_command(client, "WILL", "SGA");
	err += telnet_send_command(client, "WONT", "LINEMODE"); /* this depends on telnet client */
		
	//TODO Do we verify client response? What do we do if the response is not how we expected?
	
	return err;
}

/* Handles special characters in data buffer after receiving them from client. */
int telnet_handle_client_read(char *databuf, int *datalen) {	
	
	int i;
	char newdata[DATA_BUFLEN];
	int newlen = 0;
	
	/* process data using a new buffer */
	for (i = 0; i < *datalen; i++) {
		/* handle and discard telnet commands */
		if (databuf[i] == telnet_option_value("IAC")) {
			telnet_handle_command((databuf+i), 3);
			i += 2;
		}
		/* let other data pass through */
		else {
			newdata[newlen++] = databuf[i];
		}
	}
	/* overwrite data with new buffer */
	for (i = 0; i < newlen; i++) databuf[i] = newdata[i];
	*datalen = newlen;
	
	return 0;
}

/* Handles special characters in data buffer before sending to client. */
int telnet_handle_client_write(char *databuf, int *datalen) {
	
	int i;
	char newdata[DATA_BUFLEN];
	int newlen = 0;
	
	/* process data using a new buffer */
	for (i = 0; i < *datalen; i++) {
		/* pressed ENTER */
		if (databuf[i] == 13) {
			fprintf(stderr, "[%s] handling ENTER\n", __func__);
			newdata[newlen++] = '\r';
			newdata[newlen++] = '\n';
		}
		/* pressed BACKSPACE */
		if (databuf[i] == 127) {
			fprintf(stderr, "[%s] handling BACKSPACE\n", __func__);
			newdata[newlen++] = 8;
			newdata[newlen++] = ' ';
			newdata[newlen++] = 8;
		}
		else {
			newdata[newlen++] = databuf[i];
		}
	}
	/* overwrite data with new buffer */
	for (i = 0; i < newlen; i++) databuf[i] = newdata[i];
	*datalen = newlen;
	
	return 0;
}

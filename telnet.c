#include <telnet.h>

/* structure for holding telnet option name and value */
typedef struct
{
	const char *name;
	char value;
} telnet_option_t;

/* supported telnet option values */
telnet_option_t telnet_options[] =
{
	{"WILL", 251},
	{"WONT", 252},
	{"DO", 253},
	{"DONT", 254},
	{"IAC", 255},
	{"ECHO", 1},
	{"SGA", 3},
	{"LINEMODE", 34},
	{NULL, 0}
	/* this list must end with {NULL, 0} */
};

/* Returns the name of a telnet option based on the value. */
static const char* telnet_option_name(int value)
{
	int i;
	for (i = 0; telnet_options[i].name != NULL; i++)
	{
		if (telnet_options[i].value == value)
		{
			return telnet_options[i].name;
		}
	}
	/* default value */
	return '\0';
}

/* Returns the value of a telnet option based on the name. */
static char telnet_option_value(const char* name)
{
	int i;
	for (i = 0; telnet_options[i].name != NULL; i++)
	{
		if (strcmp(telnet_options[i].name, name) == 0)
		{
			return telnet_options[i].value;
		}
	}
	/* default value */
	return 0;
}

/* Handles a received telnet option command. */
static void telnet_handle_command(char *databuf, int datalen)
{
	/* just print received commands:
	 * we set the client, we don't adapt to client commands */
	if (databuf[0] == telnet_option_value("IAC"))
	{
		fprintf(stderr, "[%s] received %s %s\n", __func__,
				telnet_option_name(databuf[1]),
				telnet_option_name(databuf[2]));
	}
}

void telnet_message_set_character_mode(char *databuf)
{
	/* send a predefined set of commands proven to work */

	databuf[0] = telnet_option_value("IAC");
	databuf[1] = telnet_option_value("WILL");
	databuf[2] = telnet_option_value("ECHO");

	databuf[3] = telnet_option_value("IAC");
	databuf[4] = telnet_option_value("WILL");
	databuf[5] = telnet_option_value("SGA");

	/* this one really depends on telnet client */
	databuf[6] = telnet_option_value("IAC");
	databuf[7] = telnet_option_value("WONT");
	databuf[8] = telnet_option_value("LINEMODE");
		
	//TODO Do we verify client response? What do we do if the response is not how we expected?
}

void telnet_filter_client_read(char *databuf, int *datalen)
{
	int i;
	char newdata[BUFFER_LEN];
	int newlen = 0;
	
	/* process data using a new buffer */
	for (i = 0; i < *datalen; i++)
	{
		/* handle and discard telnet commands */
		if (databuf[i] == telnet_option_value("IAC"))
		{
			telnet_handle_command((databuf+i), 3);
			i += 2;
		}
		/* let other data pass through */
		else
		{
			newdata[newlen++] = databuf[i];
		}
	}
	/* overwrite the data with the new buffer */
	for (i = 0; i < newlen; i++)
	{
		databuf[i] = newdata[i];
	}
	/* update data length */
	*datalen = newlen;
}

void telnet_filter_client_write(char *databuf, int *datalen)
{
	int i;
	char newdata[BUFFER_LEN]; // TODO: maybe use realloc, this is risky
	int newlen = 0;
	
	/* process data using a new buffer */
	for (i = 0; i < *datalen; i++)
	{
		/* pressed ENTER */
		if (databuf[i] == 13)
		{
			fprintf(stderr, "[%s] handling ENTER\n", __func__);
			newdata[newlen++] = '\r';
			newdata[newlen++] = '\n';
		}
		/* pressed BACKSPACE */
		if (databuf[i] == 127)
		{
			fprintf(stderr, "[%s] handling BACKSPACE\n", __func__);
			newdata[newlen++] = 8;
			newdata[newlen++] = ' ';
			newdata[newlen++] = 8;
		}
		else {
			newdata[newlen++] = databuf[i];
		}
	}
	/* overwrite the data with the new buffer */
	for (i = 0; i < newlen; i++)
	{
		databuf[i] = newdata[i];
	}
	/* update data length */
	*datalen = newlen;
}

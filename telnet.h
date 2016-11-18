#pragma once

/* Handles details related to telnet protocol. */

#include <common.h>

#define TELNET_MSG_SIZE_CHARMODE 9

/**
 * Creates a telnet protocol message that tells client to go into "character"
 * mode. The passed data buffer must be big enough to hold the message payload
 * with the size defined by TELNET_MSG_SIZE_CHARMODE.
 * Operates directly on the passed data buffer.
 */
void telnet_message_set_character_mode(char *databuf);

/**
 * Handles special characters in the data buffer after receiving them from the
 * client. Used to filter out the handshake commands of telnet protocol.
 * Operates directly on the passed data buffer and modifies the payload length.
 */
void telnet_handle_client_read(char *databuf, int *datalen);

/**
 * Handles special characters in the data buffer before sending them to the
 * client. Used to correctly echo the characters to the telnet client.
 * Operates directly on the passed data buffer and modifies the payload length.
 */
void telnet_handle_client_write(char *databuf, int *datalen);

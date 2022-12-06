#ifndef __SERENADER_BOB_HANDLER_H__
#define __SERENADER_BOB_HANDLER_H__

#include <stdbool.h>

#include "connection.h"
#include "proto.h"

// Handles a protocol message from the CLI message,
// Does not free message
int handle_cli_msg(prot_msg_t const *msg, char **fifo_name);

// Handles a protocol message from pipe,
// Consumes the input (i.e., calls free())
//
// Returns the message to return
//
// Memory: responsibility of the caller to call free() on return
prot_msg_t *handle_pipe_msg(prot_msg_t *msg, conn_t *conn);

#endif // __SERENADER_BOB_HANDLER_H__

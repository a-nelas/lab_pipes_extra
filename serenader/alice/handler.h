#ifndef __SERENADER_ALICE_HANDLER_H__
#define __SERENADER_ALICE_HANDLER_H__

#include "connection.h"
#include "proto.h"

// Handles a protocol message, giving back the a message to send back
// Consumes the input (i.e., calls free())
//
// Memory: responsibility of the caller to call free() on return
prot_msg_t *handle_msg(prot_msg_t *msg, conn_t *conn);

#endif // __SERENADER_ALICE_HANDLER_H__

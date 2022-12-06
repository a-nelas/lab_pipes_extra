#ifndef __SERENADER_PROTO_H__
#define __SERENADER_PROTO_H__

#include <stddef.h>
#include <stdint.h>

typedef enum prot_code_t {
    PROT_CODE_PLAY = 1,
    PROT_CODE_STOP = 2,
    PROT_CODE_PAUSE = 3,
    PROT_CODE_REPEAT = 4,
    PROT_CODE_SHUFFLE = 5,
    PROT_CODE_SERENADE = 6,
    PROT_CODE_GOODBYE = 7,
    PROT_CODE_FATAL_ERROR = 0xfffe,
    PROT_CODE_ERROR = 0xffff,
} prot_code_t;

// This struct can leverage something called FAM: flexible member array.
//
// In C, it is legal to let the last member of a struct be an array with
// unspecified size, as m_msg is here.
//
// This adds a lot of flexibility. Once we know the size of the message, in
// this case m_size, we can allocate the full message with the size of the
// message, like so:
//
// ```
// size_t size = SOME_VALUE'
// msg_t * msg = malloc( sizeof(msg_t) + size * sizeof(uint8_t));
//
// msg->m_code = PROT_CODE_UNKNOWN;
// msg->m_size = size;
//
// ... // initialize msg->m_msg somehow
//
// puts(msg->m_msg)
// ```
//
// note that in this case sizeof(msg_t) yields the size of the struct without
// the size of the flexible member array.
//
// This structure will be extremely helpful when parsing.
typedef struct prot_msg_t {
    prot_code_t m_code;
    uint64_t m_size;
    uint8_t m_msg[];
} prot_msg_t;

// helpers to handle sending/receiving protocol messages

// Responsibility of caller to make sure reading fd will not block
//
// MEMORY: responsability of the caller to free()
prot_msg_t *prot_recv(int fd, bool wants_fd_read, bool *eof);

// send a message through fd
// consumes message, calling free()
int prot_send(int fd, prot_msg_t *msg);

// helpers to create protocol messages
//
// MEMORY: responsability of the caller to free()
prot_msg_t *prot_create_play(char const *nickname);
prot_msg_t *prot_create_stop();
prot_msg_t *prot_create_pause();
prot_msg_t *prot_create_repeat();
prot_msg_t *prot_create_shuffle();

prot_msg_t *prot_create_serenade(char const *segment);
prot_msg_t *prot_create_goodbye(char const *message);
prot_msg_t *prot_create_error(char const *message);
prot_msg_t *prot_create_fatal_error(char const *message);

// frees any intermediate state from the library
void prot_free();

#endif // __SERENADER_PROTO_H__

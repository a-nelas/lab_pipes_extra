#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "logging.h"
#include "proto.h"

#undef ERROR_MSG_SIZE
#define ERROR_MSG_SIZE (4096)

#undef BUFFER_SIZE
#define BUFFER_SIZE (4096)

// ========================
//  helpers
// ========================
static prot_msg_t *prot_create_short(prot_code_t code) {
    prot_msg_t *msg = malloc(sizeof(prot_msg_t));
    if (msg == NULL) {
        WARN("failed to allocate %zu bytes: %s", sizeof(prot_msg_t),
             strerror(errno));
        return NULL;
    }

    msg->m_code = code;
    msg->m_size = 0;
    return msg;
}

static prot_msg_t *prot_create_long(prot_code_t code, char const *message,
                                    size_t message_len) {
    // leave an extra byte for null byte at the end
    // calloc also ensures that this is zeroed out
    prot_msg_t *msg = calloc(1, sizeof(prot_msg_t) + message_len + 1);
    if (msg == NULL) {
        WARN("failed to allocate %zu bytes: %s",
             sizeof(prot_msg_t) + message_len, strerror(errno));
        return NULL;
    }

    msg->m_code = code;
    msg->m_size = message_len;
    memcpy(msg->m_msg, message, message_len);
    return msg;
}

// message that is being parsed
prot_msg_t *g_curr_msg = NULL;
uint64_t g_curr_msg_parsed = 0;

static prot_msg_t *prot_continue_parsing(uint8_t const *buffer,
                                         size_t buffer_size,
                                         size_t *n_bytes_read) {
    uint64_t to_parse = g_curr_msg->m_size - g_curr_msg_parsed;
    if (to_parse < buffer_size) {
        to_parse = buffer_size;
    }
    memcpy(g_curr_msg->m_msg + g_curr_msg_parsed, buffer, to_parse);
    g_curr_msg_parsed += to_parse;
    *n_bytes_read += to_parse;

    // we finished parsing this message
    if (g_curr_msg->m_size == g_curr_msg_parsed) {

        prot_msg_t *ret = g_curr_msg;
        g_curr_msg = NULL;
        g_curr_msg_parsed = 0;

        return ret;
    }
    return NULL;
}

static prot_msg_t *prot_start_parsing_long(prot_code_t code,
                                           uint8_t const *buffer,
                                           size_t buffer_size,
                                           size_t *n_bytes_read) {
    if (buffer_size < sizeof(uint8_t) + sizeof(uint64_t)) {
        // not even ready to start parsing
        *n_bytes_read = 0;
        return NULL;
    }

    uint64_t message_size = 0;
    memcpy(&message_size, buffer + *n_bytes_read, sizeof(uint64_t));
    *n_bytes_read += sizeof(uint64_t);

    // self-contained message: parse oneshot
    if (message_size <= buffer_size - *n_bytes_read) {
        prot_msg_t *msg = prot_create_long(
            code, (char const *)(buffer + *n_bytes_read), message_size);
        *n_bytes_read += message_size;
        return msg;
    }

    // long message, not contained in buffer
    // start parsing
    g_curr_msg = malloc(sizeof(prot_msg_t) + message_size);
    if (g_curr_msg == NULL) {
        WARN("failed to allocate %zu bytes: %s",
             sizeof(prot_msg_t) + message_size, strerror(errno));
        return NULL;
    }

    g_curr_msg->m_code = code;
    g_curr_msg->m_size = message_size;
    memcpy(g_curr_msg->m_msg, buffer + *n_bytes_read,
           buffer_size - *n_bytes_read);
    g_curr_msg_parsed = buffer_size - *n_bytes_read;
    *n_bytes_read = buffer_size;
    return NULL;
}

// protocol parser: yields a message when it is formed,
// returning how much of the buffer was read via the n_bytes_read argument
//
// MEMORY: responsability of the caller to free()
static prot_msg_t *prot_parse(uint8_t const *buffer, size_t buffer_size,
                              size_t *n_bytes_read) {
    if (buffer_size < sizeof(uint8_t)) {
        *n_bytes_read = 0;
        return NULL;
    }

    // there is a message being parsed
    if (g_curr_msg != NULL) {
        return prot_continue_parsing(buffer, buffer_size, n_bytes_read);
    }

    // parsing a new message
    prot_code_t code = (prot_code_t)buffer[0];
    *n_bytes_read = sizeof(uint8_t);
    switch (code) {
    case PROT_CODE_STOP:
        *n_bytes_read += sizeof(uint64_t);
        return prot_create_stop();
    case PROT_CODE_PAUSE:
        *n_bytes_read += sizeof(uint64_t);
        return prot_create_pause();
    case PROT_CODE_REPEAT:
        *n_bytes_read += sizeof(uint64_t);
        return prot_create_repeat();
    case PROT_CODE_SHUFFLE:
        *n_bytes_read += sizeof(uint64_t);
        return prot_create_shuffle();

    case PROT_CODE_PLAY:
    case PROT_CODE_SERENADE:
    case PROT_CODE_GOODBYE:
    case PROT_CODE_FATAL_ERROR:
    case PROT_CODE_ERROR:
        return prot_start_parsing_long(code, buffer, buffer_size, n_bytes_read);
    default: {
        char error_msg[ERROR_MSG_SIZE];
        snprintf(error_msg, ERROR_MSG_SIZE, "unknown message code: %d | %x",
                 code, code);
        return prot_create_error(error_msg);
    }
    }
}

// ========================
//  library implementation
// ========================

uint8_t g_read_buffer[BUFFER_SIZE];

size_t g_read_buffer_read = 0;

size_t g_read_buffer_processed = 0;

static prot_msg_t *prot_process() {
    prot_msg_t *msg = NULL;
    if (g_read_buffer_processed < g_read_buffer_read) {
        size_t bytes_read = 0;
        msg = prot_parse(g_read_buffer + g_read_buffer_processed,
                         g_read_buffer_read - g_read_buffer_processed,
                         &bytes_read);
        g_read_buffer_processed += bytes_read;
    }

    if (g_read_buffer_read == g_read_buffer_processed) {
        g_read_buffer_read = 0;
        g_read_buffer_processed = 0;
    }

    if (msg == NULL && g_read_buffer_read > BUFFER_SIZE - 16) {
        // if we 1) cannot parse a message and 2) are close to the end of the
        // buffer then we might need to make more space for new messages

        // shift into the beginning of the buffer
        memmove(g_read_buffer, g_read_buffer + g_read_buffer_processed,
                g_read_buffer_read - g_read_buffer_processed);

        g_read_buffer_read -= g_read_buffer_processed;
        g_read_buffer_processed = 0;
    }

    return msg;
}

// MEMORY: responsability of the caller to free()
prot_msg_t *prot_recv(int fd, bool wants_fd_read, bool *eof) {
    // TO IMPLEMENT
    return NULL;
}

// send a message through fd
// consumes message, calling free()
int prot_send(int fd, prot_msg_t *msg) {
    // TO IMPLEMENT
    return -1;
}

prot_msg_t *prot_create_play(char const *nickname) {
    return prot_create_long(PROT_CODE_PLAY, nickname, strlen(nickname));
}
prot_msg_t *prot_create_stop() { return prot_create_short(PROT_CODE_STOP); }
prot_msg_t *prot_create_pause() { return prot_create_short(PROT_CODE_PAUSE); }
prot_msg_t *prot_create_repeat() { return prot_create_short(PROT_CODE_REPEAT); }
prot_msg_t *prot_create_shuffle() {
    return prot_create_short(PROT_CODE_SHUFFLE);
}

prot_msg_t *prot_create_serenade(char const *segment) {
    return prot_create_long(PROT_CODE_SERENADE, segment, strlen(segment));
}
prot_msg_t *prot_create_goodbye(char const *message) {
    return prot_create_long(PROT_CODE_GOODBYE, message, strlen(message));
}
prot_msg_t *prot_create_fatal_error(char const *message) {
    return prot_create_long(PROT_CODE_FATAL_ERROR, message, strlen(message));
}
prot_msg_t *prot_create_error(char const *message) {
    return prot_create_long(PROT_CODE_ERROR, message, strlen(message));
}

void prot_free() {
    if (g_curr_msg != NULL) {
        free(g_curr_msg);
    }
    g_curr_msg = NULL;
    g_curr_msg_parsed = 0;
}

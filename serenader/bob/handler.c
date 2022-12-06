#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "cli.h"
#include "handler.h"
#include "logging.h"

#undef VERSE_TIMEOUT_MS
#define VERSE_TIMEOUT_MS (1000)

#undef FILENAME_SIZE
#define FILENAME_SIZE (2048)

char g_recv_fifo_name[FILENAME_SIZE];

// Handle a cli message from
int handle_cli_msg(prot_msg_t const *command, char **fifo_name) {
    *fifo_name = NULL;
    if (command != NULL) {
        switch (command->m_code) {
        case PROT_CODE_PLAY:
            memset(g_recv_fifo_name, 0, FILENAME_SIZE);
            memcpy(g_recv_fifo_name, command->m_msg,
                   (command->m_size < FILENAME_SIZE - 1) ? command->m_size
                                                         : FILENAME_SIZE - 1);
            *fifo_name = g_recv_fifo_name;
            break;
        case PROT_CODE_STOP:
            break;
        case PROT_CODE_ERROR:
            WARN("error: %.*s", (int)command->m_size, command->m_msg);
            break;
        case PROT_CODE_FATAL_ERROR:
            WARN("fatal error: %.*s", (int)command->m_size, command->m_msg);
            return -1;
        case PROT_CODE_PAUSE:
            break;
        case PROT_CODE_REPEAT:
            break;
        case PROT_CODE_SHUFFLE:
            break;
        case PROT_CODE_SERENADE:
            WARN("unexpected CLI command: serenade");
            break;
        case PROT_CODE_GOODBYE:
            WARN("unexpected CLI command: goodbye");
            break;
        }
    }

    return 0;
}

// Handle a pipe message from Alice
prot_msg_t *handle_pipe_msg(prot_msg_t *msg, conn_t *conn) {
    if (msg != NULL) {
        switch (msg->m_code) {
        case PROT_CODE_PLAY:
            fprintf(stderr, "%s:%d :: %s :: unexpected play from alice\n",
                    __FILE__, __LINE__, __func__);
            break;
        case PROT_CODE_STOP:
            fprintf(stderr, "%s:%d :: %s :: unexpected stop from alice\n",
                    __FILE__, __LINE__, __func__);
            break;
        case PROT_CODE_PAUSE:
            fprintf(stderr, "%s:%d :: %s :: unexpected pause from alice\n",
                    __FILE__, __LINE__, __func__);
            break;
        case PROT_CODE_REPEAT:
            fprintf(stderr, "%s:%d :: %s :: unexpected repeat from alice\n",
                    __FILE__, __LINE__, __func__);
            break;
        case PROT_CODE_SHUFFLE:
            fprintf(stderr, "%s:%d :: %s :: unexpected shuffle from alice\n",
                    __FILE__, __LINE__, __func__);
            break;
        case PROT_CODE_SERENADE:
            cli_clear_prompt();
            fprintf(stdout, "%.*s\n", (int)msg->m_size, msg->m_msg);
            break;
        case PROT_CODE_GOODBYE:
            fprintf(stdout, "alice says: %.*s\n", (int)msg->m_size, msg->m_msg);
            conn_close(conn);
            break;
        case PROT_CODE_FATAL_ERROR:
            fprintf(stderr, "fatal error: %.*s\n", (int)msg->m_size,
                    msg->m_msg);
            conn_close(conn);
            break;
        case PROT_CODE_ERROR:
            fprintf(stderr, "error: %.*s\n", (int)msg->m_size, msg->m_msg);
            break;
        }
        free(msg);
    }

    return NULL;
}

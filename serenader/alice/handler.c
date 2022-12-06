#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fifo.h"
#include "handler.h"
#include "logging.h"
#include "playlist.h"

#undef ERROR_SIZE
#define ERROR_SIZE (2048)

#undef FILENAME_SIZE
#define FILENAME_SIZE (2048)

// Handle message from Bob
// Central function for implementation
//
// In this function, we can return not only replies,
// but song verses as well
prot_msg_t *handle_msg(prot_msg_t *msg, conn_t *conn) {
    if (msg != NULL) {
        switch (msg->m_code) {
        case PROT_CODE_PLAY:
            if (playlist_play((char const *)msg->m_msg, msg->m_size) != 0) {

                WARN(
                    "couldn't start serenading %.*s, I'm already serenading %s",
                    (int)msg->m_size, msg->m_msg, playlist_current_nickname());
                break;
            }
            char send_pipe_name[FILENAME_SIZE];
            memset(send_pipe_name, 0, FILENAME_SIZE);
            memcpy(send_pipe_name, msg->m_msg,
                   (msg->m_size < FILENAME_SIZE - 1) ? msg->m_size
                                                     : FILENAME_SIZE - 1);

            if (conn_begin(conn, send_pipe_name) == -1) {
                WARN("failed to establish %s: %s", send_pipe_name,
                     strerror(errno));
            }
            break;
        case PROT_CODE_STOP: {
            char const *nickname = playlist_current_nickname();
            if (nickname == NULL) {
                free(msg);
                return prot_create_error("cannot stop when nothing is playing");
            }

            prot_msg_t *return_msg = prot_create_goodbye(nickname);
            playlist_stop();
            free(msg);
            return return_msg;
        }
        case PROT_CODE_PAUSE:
            playlist_pause();
            break;
        case PROT_CODE_REPEAT:
            playlist_toggle_repeat();
            break;
        case PROT_CODE_SHUFFLE:
            playlist_toggle_shuffle();
            break;
        case PROT_CODE_SERENADE:
            WARN("unexpected serenade from bob");
            break;
        case PROT_CODE_GOODBYE:
            WARN("unexpected goodbye from bob");
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

    char const *verse = playlist_next_verse();
    if (verse != NULL) {
        return prot_create_serenade(verse);
    }

    return NULL;
}

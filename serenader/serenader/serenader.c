#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "cli.h"
#include "logging.h"
#include "playlist.h"

// return false to quit
bool execute_command(prot_msg_t *cmd) {
    bool keep_going = true;
    if (cmd != NULL) {
        LOG("executing command: %d", cmd->m_code);
        switch (cmd->m_code) {
        case PROT_CODE_PLAY:
            if (playlist_play((char const *)cmd->m_msg, cmd->m_size) == -1) {
                WARN("failed to init playlist");
                return -1;
            }
            break;
        case PROT_CODE_STOP:
            playlist_stop();
            keep_going = false;
            break;
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
            break;
        case PROT_CODE_GOODBYE:
            break;
        case PROT_CODE_FATAL_ERROR:
            fprintf(stderr, "fatal error: %.*s\n", (int)cmd->m_size,
                    cmd->m_msg);
            free(cmd);
            exit(EXIT_FAILURE);
            break;
        case PROT_CODE_ERROR:
            fprintf(stderr, "error: %.*s\n", (int)cmd->m_size, cmd->m_msg);
            break;
        }
        free(cmd);
    }
    return keep_going;
}

int main() {
    set_log_level(LOG_VERBOSE);

    bool keep_going = true;
    do {
        prot_msg_t *cli_msg = NULL;
        cli_print_prompt();

        fd_set readfds;
        fd_set exceptfds;
        FD_ZERO(&readfds);
        FD_ZERO(&exceptfds);

        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(STDIN_FILENO, &exceptfds);

        // timeout of 0 makes it return immediately
        struct timeval timeout = {.tv_sec = 0, .tv_usec = 0};

        int select_ret =
            select(STDIN_FILENO + 1, &readfds, NULL, &exceptfds, &timeout);
        switch (select_ret) {
        case 0: // timeout
            break;

        case -1: // error
            WARN("failed to select: %s", strerror(errno));
            break;

        default: // some activity
            if (FD_ISSET(STDIN_FILENO, &exceptfds)) {
                WARN("stdin failure : %s", strerror(errno));
            } else if (FD_ISSET(STDIN_FILENO, &readfds)) {
                cli_msg = cli_read();
                cli_clear_prompt();
            }
        }

        keep_going = execute_command(cli_msg);

        char const *verse = playlist_next_verse();
        if (keep_going && verse != NULL) {
            cli_clear_prompt();
            fprintf(stdout, "%s\n", verse);
        }
    } while (keep_going);

    return 0;
}

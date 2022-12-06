#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "cli.h"
#include "connection.h"
#include "fifo.h"
#include "handler.h"
#include "logging.h"
#include "proto.h"

#undef DEFAULT_SEND_FIFO
#define DEFAULT_SEND_FIFO "alice.fifo"

#undef FILENAME_SZ
#define FILENAME_SZ 4096

static char const *g_send_fifo_name = DEFAULT_SEND_FIFO;

static char g_recv_fifo_name[FILENAME_SZ];
static bool g_created_recv_fifo = false;

static conn_t g_conn;

static int process_cli(int argc, char **argv) {
    if (argc == 3) {
        // rename fifos
        g_send_fifo_name = argv[1];
    } else if (argc != 1) {
        fprintf(stderr,
                "usage: bob <send_fifo> <recv_fifo>\n"
                "   <send_fifo> (default = %s): filename for the FIFO bob will "
                "use to send messages to alice\n",
                g_send_fifo_name);
        return -1;
    }

    set_log_level(LOG_QUIET);
    return 0;
}

int main(int argc, char **argv) {
    if (process_cli(argc, argv) != 0) {
        return -1;
    }

    // file descriptor for tx fifo
    int tx = open_send_fifo(g_send_fifo_name, false);
    if (tx == -1) {
        WARN("failed to send receive pipe");
        return -1;
    } else if (conn_create(&g_conn, tx) == -1) {
        WARN("failed to create connection");
        return -1;
    }

    do {
        prot_msg_t *recvd_msg = NULL;
        prot_msg_t *cli_msg = NULL;
        bool rx_eof = false;
        bool wants_fd_read = false;

        cli_print_prompt();

        fd_set readfds;
        fd_set exceptfds;
        FD_ZERO(&readfds);
        FD_ZERO(&exceptfds);

        if (g_conn.rx != -1) {
            FD_SET(g_conn.rx, &readfds);
            FD_SET(g_conn.rx, &exceptfds);
        }
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(STDIN_FILENO, &exceptfds);

        // timeout of 0 makes it return immediately
        struct timeval timeout = {.tv_sec = 0, .tv_usec = 0};

        int max_fd = (g_conn.rx > STDIN_FILENO) ? g_conn.rx : STDIN_FILENO;
        int select_ret =
            select(max_fd + 1, &readfds, NULL, &exceptfds, &timeout);
        switch (select_ret) {
        case 0: // timeout
            break;

        case -1: // error
            WARN("failed to select: %s", strerror(errno));
            break;

        default: // some activity
            if (g_conn.rx != -1 && FD_ISSET(g_conn.rx, &exceptfds)) {
                WARN("pipe failure : %s", strerror(errno));
            } else if (g_conn.rx != -1 && FD_ISSET(g_conn.rx, &readfds)) {
                wants_fd_read = true;
            }

            if (FD_ISSET(STDIN_FILENO, &exceptfds)) {
                WARN("stdin failure : %s", strerror(errno));
            } else if (FD_ISSET(STDIN_FILENO, &readfds)) {
                cli_msg = cli_read();
                cli_clear_prompt();
            }
        }

        char *fifo_name = NULL;
        if (handle_cli_msg(cli_msg, &fifo_name) == -1) {
            free(cli_msg);
            conn_close(&g_conn);
            continue;
        }

        recvd_msg = prot_recv(g_conn.rx, wants_fd_read, &rx_eof);
        prot_msg_t *reply = handle_pipe_msg(recvd_msg, &g_conn);

        bool is_stop = (cli_msg != NULL) && cli_msg->m_code == PROT_CODE_STOP;
        if (prot_send(g_conn.tx, cli_msg) != 0) {
            WARN("failed to send command: %s", strerror(errno));
        }

        if (is_stop) {
            if (conn_sent_stop(&g_conn) == -1) {
                WARN("failed to mark connection after sending stop");
                return -1;
            }
        }

        if (fifo_name != NULL) {
            memset(g_recv_fifo_name, 0, FILENAME_SZ);
            strncpy(g_recv_fifo_name, fifo_name, FILENAME_SZ);
            if (conn_begin(&g_conn, fifo_name) == -1) {
                break;
            }
            g_created_recv_fifo = true;
        }

        if (rx_eof) {
            conn_close(&g_conn);
        }
    } while (g_conn.status != CONN_CLOSED);

    prot_free();

    if (g_created_recv_fifo) {
        if (unlink(g_recv_fifo_name) != 0) {
            WARN("failed to unlink %s: %s", g_recv_fifo_name, strerror(errno));
        }
    }

    return 0;
}

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "connection.h"
#include "fifo.h"
#include "handler.h"
#include "logging.h"
#include "proto.h"

#undef DEFAULT_RECV_FIFO
#define DEFAULT_RECV_FIFO "alice.fifo"

char const *g_recv_fifo_name = DEFAULT_RECV_FIFO;

conn_t g_conn;

static int process_cli(int argc, char **argv) {
    if (argc == 2) {
        // rename fifos
        g_recv_fifo_name = argv[1];
    } else if (argc != 1) {
        fprintf(stderr,
                "usage: alice <recv_fifo>\n"
                "   <recv_fifo> (default = %s): filename for the FIFO bob will "
                "use to receive messages from alice\n",
                g_recv_fifo_name);
        return -1;
    }

    set_log_level(LOG_VERBOSE);
    return 0;
}

int main(int argc, char **argv) {
    if (process_cli(argc, argv) != 0) {
        return -1;
    }

    while (true) {

        int rx = open_recv_fifo(g_recv_fifo_name, true);
        if (conn_create(&g_conn, rx) == -1) {
            WARN("failed to create connection");
            return -1;
        }

        while (g_conn.status != CONN_CLOSED) {
            bool rx_eof = false;
            bool wants_fd_read = false;
            fd_set readfds;
            fd_set exceptfds;
            FD_ZERO(&readfds);
            FD_ZERO(&exceptfds);

            if (g_conn.rx != -1) {
                FD_SET(g_conn.rx, &readfds);
                FD_SET(g_conn.rx, &exceptfds);
            }
            // timeout of 0 makes it return immediately
            struct timeval timeout = {.tv_sec = 0, .tv_usec = 0};

            int max_fd = (g_conn.rx >= 0) ? g_conn.rx : -1;

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
            }

            prot_msg_t *recvd_msg =
                prot_recv(g_conn.rx, wants_fd_read, &rx_eof);
            prot_msg_t *send_msg = handle_msg(recvd_msg, &g_conn);
            bool is_goodbye =
                send_msg != NULL && send_msg->m_code == PROT_CODE_STOP;

            if (rx_eof) {
                conn_close(&g_conn);
            }

            if (send_msg != NULL && g_conn.tx != -1) {
                if (prot_send(g_conn.tx, send_msg) != 0) {
                    WARN("failed to send protocol message: %s",
                         strerror(errno));
                }
            }

            if (is_goodbye) {
                conn_close(&g_conn);
            }
        }
    }

    return 0;
}

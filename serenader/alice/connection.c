#include <unistd.h>

#include "connection.h"
#include "fifo.h"
#include "logging.h"

int conn_create(conn_t *conn, int rx) {
    conn->rx = rx;
    conn->tx = -1;
    conn->status = CONN_BEGIN;
    return 0;
}

int conn_begin(conn_t *conn, char *fifo_name) {
    if (conn->status != CONN_BEGIN) {
        WARN("connection status is not begin");
        return -1;
    }

    conn->tx = open_send_fifo(fifo_name, false);
    if (conn->tx == -1) {
        WARN("failed to create send pipe");
        conn_close(conn);
        return -1;
    }

    conn->status = CONN_ESTABLISHED;
    return 0;
}

// close the remaining
int conn_close(conn_t *conn) {
    if (conn->tx != -1) {
        close(conn->tx);
        conn->tx = -1;
    }
    if (conn->rx != -1) {
        close(conn->rx);
        conn->rx = -1;
    }

    conn->status = CONN_CLOSED;
    return 0;
}

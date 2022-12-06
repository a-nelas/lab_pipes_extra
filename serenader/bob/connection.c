#include <unistd.h>

#include "connection.h"
#include "fifo.h"
#include "logging.h"

int conn_create(conn_t *conn, int tx) {
    conn->rx = -1;
    conn->tx = tx;
    conn->status = CONN_BEGIN;
    return 0;
}

// when we begin, we add the tx and the filename to open
// when this finished, the conn is established
int conn_begin(conn_t *conn, char *fifo_name) {
    if (conn->status != CONN_BEGIN) {
        WARN("connection status is not begin");
        return -1;
    }

    conn->rx = open_recv_fifo(fifo_name, true);
    if (conn->rx == -1) {
        WARN("failed to create recv pipe");
        conn_close(conn);
        return -1;
    }

    conn->status = CONN_ESTABLISHED;
    return 0;
}

// once we send the stop, no more messages are sent, so we close the tx
int conn_sent_stop(conn_t *conn) {
    if (conn->status != CONN_ESTABLISHED) {
        WARN("connection status is not established");
        return -1;
    }

    if (conn->tx != -1) {
        close(conn->tx);
        conn->tx = -1;
    }

    conn->status = CONN_WAITING_FOR_GOODBYE;
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

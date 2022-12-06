#ifndef __SERENADER_BOB_CONN_H__
#define __SERENADER_BOB_CONN_H__

typedef enum {
    CONN_CLOSED,
    CONN_BEGIN,
    CONN_ESTABLISHED,
    CONN_WAITING_FOR_GOODBYE,
} conn_status_t;

typedef struct {
    int rx;
    int tx;
    conn_status_t status;
} conn_t;

int conn_create(conn_t *conn, int tx);

// when we begin, we add the tx and the filename to open
// when this finished, the conn is established
int conn_begin(conn_t *conn, char *fifo_name);

// once we send the stop, no more messages are sent, so we close the tx
int conn_sent_stop(conn_t *conn);

// close the remaining
int conn_close(conn_t *conn);

#endif // __SERENADER_BOB_CONN_H__

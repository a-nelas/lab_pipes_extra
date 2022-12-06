#ifndef __SERENADER_ALICE_CONN_H__
#define __SERENADER_ALICE_CONN_H__

typedef enum {
    CONN_CLOSED,
    CONN_BEGIN,
    CONN_ESTABLISHED,
} conn_status_t;

typedef struct {
    int rx;
    int tx;
    conn_status_t status;
} conn_t;

int conn_create(conn_t *conn, int rx);

// when we begin, give the filename to open
int conn_begin(conn_t *conn, char *fifo_name);

// close the remaining
int conn_close(conn_t *conn);

#endif // __SERENADER_ALICE_CONN_H__

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "fifo.h"
#include "logging.h"

int open_send_fifo(char const *fifo_filename, bool create) {
    // TO IMPLEMENT
    return -1;
}

int open_recv_fifo(char const *fifo_filename, bool create) {
    // TO IMPLEMENT
    return -1;
}

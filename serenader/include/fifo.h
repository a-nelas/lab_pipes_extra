#ifndef __SERENADER_FIFO_H__
#define __SERENADER_FIFO_H__

#include <stdbool.h>

int open_send_fifo(char const *fifo_filename, bool create);
int open_recv_fifo(char const *fifo_filename, bool create);

#endif // __SERENADER_FIFO_H__

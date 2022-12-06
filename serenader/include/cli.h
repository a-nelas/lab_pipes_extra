#ifndef __SERENADER_CLI_H__
#define __SERENADER_CLI_H__

#include <stdbool.h>

#include "proto.h"

void cli_print_prompt(void);
void cli_clear_prompt(void);

// Responsibility of caller to make sure that STDIN is ready for read
//
// Memory: caller must free
prot_msg_t *cli_read(void);

#endif // __SERENADER_CLI_H__

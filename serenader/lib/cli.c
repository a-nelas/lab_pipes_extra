#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "cli.h"
#include "logging.h"
#include "playlist.h"

#undef LINE_READ
#define LINE_READ (2048)

#undef ERROR_MSG_SIZE
#define ERROR_MSG_SIZE (1024)

// ========================
//  helper
// ========================

static prot_msg_t *parse_error(char const *file, char const *func, int lineno,
                               char const *errormsg, char const *line) {
    prot_msg_t *msg =
        calloc(1, sizeof(prot_msg_t) + LINE_READ + ERROR_MSG_SIZE);
    if (msg == NULL) {
        WARN("failed to alloc message: %s", strerror(errno));
        return NULL;
    }
    msg->m_code = PROT_CODE_ERROR;
    msg->m_size =
        (uint64_t)snprintf((char *)msg->m_msg, LINE_READ + ERROR_MSG_SIZE,
                           "%s:%d :: %s :: failed to parse : %s : %s", file,
                           lineno, func, errormsg, line);
    return msg;
}

// CLI:
//
// > play <nickname>
// > stop
// > pause
// > repeat
// > shuffle
static prot_msg_t *parse_line(char const *line, size_t line_sz) {
    // empty line, just return without making a fuss about it
    if (line_sz == 0) {
        return NULL;
    }

    if (line_sz > 5 && strncmp(line, "play", 4) == 0) {
        size_t nick_sz = line_sz - 5;
        prot_msg_t *play_msg = malloc(sizeof(prot_msg_t) + nick_sz);
        if (play_msg == NULL) {
            WARN("failed to alloc message: %s", strerror(errno));
            return NULL;
        }
        play_msg->m_code = PROT_CODE_PLAY;
        play_msg->m_size = nick_sz;
        memcpy(play_msg->m_msg, line + 5, nick_sz);
        return play_msg;
    }

    prot_code_t code = PROT_CODE_ERROR;
    if (line_sz < 4) {
        return parse_error(__FILE__, __func__, __LINE__, "small message", line);
    } else if (strncmp(line, "stop", 4) == 0) {
        code = PROT_CODE_STOP;
    } else if (line_sz < 5) {
        return parse_error(__FILE__, __func__, __LINE__, "small message", line);
    } else if (strncmp(line, "pause", 5) == 0) {
        code = PROT_CODE_PAUSE;
    } else if (line_sz < 6) {
        return parse_error(__FILE__, __func__, __LINE__, "small message", line);
    } else if (strncmp(line, "repeat", 6) == 0) {
        code = PROT_CODE_REPEAT;
    } else if (line_sz < 7) {
        return parse_error(__FILE__, __func__, __LINE__, "small message", line);
    } else if (strncmp(line, "shuffle", 7) == 0) {
        code = PROT_CODE_SHUFFLE;
    }

    if (code == PROT_CODE_ERROR) {
        return parse_error(__FILE__, __func__, __LINE__, "invalid message",
                           line);
    }

    prot_msg_t *msg = malloc(sizeof(prot_msg_t));
    if (msg == NULL) {
        WARN("failed to alloc message: %s", strerror(errno));
        return NULL;
    }

    msg->m_code = code;
    msg->m_size = 0;
    return msg;
}

// ========================
//  library implementation
// ========================

static bool g_cli_displayed = false;

void cli_print_prompt() {
    if (!g_cli_displayed) {
        g_cli_displayed = true;
        fprintf(stdout, "> ");
        fflush(stdout);
    }
}

void cli_clear_prompt() {
    if (g_cli_displayed) {
        g_cli_displayed = false;
        fprintf(stdout, "\r");
        fflush(stdout);
    }
}

prot_msg_t *cli_read() {
    char line[LINE_READ];
    memset(line, 0, LINE_READ);
    if (fgets(line, LINE_READ - 1, stdin) == NULL) {
        if (feof(stdin)) { // Ctrl-D works as STOP
            return prot_create_stop();
        }

        WARN("failed to fgets : %s", strerror(errno));
        return NULL;
    }

    size_t len = strlen(line);
    if (line[len - 1] == '\n') {
        line[len - 1] = '\0';
        len -= 1;
    }
    return parse_line(line, len);
}

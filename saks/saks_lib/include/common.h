#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <syslog.h>
#include <stdint.h>
#include <stdbool.h>

extern bool use_syslog;

#define LOGD(fmt, ...) do {                                              \
    if (use_syslog) {                                                    \
    } else {                                                             \
        printf("D/(%s,%d): "fmt"\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        fflush(stdout);                                                  \
    }                                                                    \
} while (0)

#define LOGE(fmt, ...) do {                    \
    if (use_syslog) {                          \
        syslog(LOG_ERR, fmt, ##__VA_ARGS__);   \
    } else {                                   \
        printf("E/"fmt"\n", ##__VA_ARGS__);    \
        fflush(stdout);                        \
    }                                          \
} while (0)

#define LOGI(fmt, ...) do {                    \
    if (use_syslog) {                          \
        syslog(LOG_INFO, fmt, ##__VA_ARGS__);  \
    } else {                                   \
        printf("I/"fmt"\n", ##__VA_ARGS__);    \
        fflush(stdout);                        \
    }                                          \
} while (0)

extern char **strsplit(const char *str, const char *delim, size_t *numtokens);
extern void tokensfree(char **tokens, size_t numtokens);

#endif /* end of include guard: COMMON_H */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "digiled.h"

static void usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s [-t TOGGLE_STATE] [-i INTEGER] [-r ROTATE_SPEED]\n"
            "   -t toggle state\n"
            "       3 - enable all display\n"
            "       2 - only digital display\n"
            "       1 - only rotation display\n"
            "       0 - disable all display\n"
            "   -i integer\n"
            "       effective range is 0 ~ 999000\n"
            "   -r rotate speed\n"
            "       1 ~ 10\n"
            "\n", prog
            );
}

int main(int argc, const char *argv[])
{
    char buf[128];
    int fd;
    int n;
    int i;

    if (argc == 1) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(buf, 0, sizeof(buf));
    fd = open("/proc/self/cmdline", O_RDONLY);
    n = read(fd, buf, 128);
    close(fd);

    for (i = 0; i < n; i++) {
        if (buf[i] == '\0') {
            buf[i] = ' ';
        }
    }
    buf[n - 1] = '\0';

    dl_send_msg(buf);

    return 0;
}

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>

#include "digiled.h"
#include "usock.h"

int dl_send_msg(char *msg)
{
    int sockfd;
    struct sockaddr_un servaddr;

    sockfd = usock(USOCK_UNIX, DIGILED_UNIX_SOCKET, NULL);

    if (write(sockfd, msg, strlen(msg) + 1) < 0) {
        perror("write");
        return errno;
    }
}

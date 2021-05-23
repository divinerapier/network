#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef __MAXLINE__
#define MAXLINE 4096
#endif

static void errorf(const char *format, ...) {
    static char *buffer;
    static size_t buffer_size;

    va_list argptr;
    va_start(argptr, format);
    int length = vsnprintf(buffer, buffer_size, format, argptr);
    va_end(argptr);

    if (length + 1 > buffer_size) {
        buffer_size = length + 1;
        buffer = realloc(buffer, buffer_size);
        /* Yes, `realloc` should be done differently to properly handle
           possible failures. But that's beside the point in this context */

        va_start(argptr, format);
        vsnprintf(buffer, buffer_size, format, argptr);
        va_end(argptr);
    }

    perror(buffer);
}

int main() {
    int ret;
    int listenfd, connfd;
    socklen_t len;
    struct sockaddr_in servaddr, cliaddr;
    char buf[MAXLINE] = {0};
    time_t ticks;

    // man 2 socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 != errno) {
        errorf("open a socket file with %d.", errno);
        return errno;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 0.0.0.0
    servaddr.sin_port = htons(9999); // convert host long/short byte order to network order

    if (0 != (ret = bind(listenfd, (const struct sockaddr *) &servaddr, sizeof(servaddr)))) {
        char host[INET_ADDRSTRLEN] = {0};
        errorf("bind on socket: %s:%d", inet_ntop(AF_INET, INADDR_ANY, host, INET_ADDRSTRLEN), 9999);
        return errno;
    }
    char *host = inet_ntoa(servaddr.sin_addr);
    if (0 != errno) {
        errorf("convert numeric host to presentation host.");
        return errno;
    }
    printf("succeed to bind on socket: %s:%d\n", host, 9999);

    listen(listenfd, 0);

    while (1) {
        len = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len);
        if (0 != errno) {
            errorf("accept error");
            return errno;
        }
        printf("connection from %s:%d\n",
               inet_ntop(AF_INET, &cliaddr.sin_addr, buf, sizeof(buf)),
               ntohs(cliaddr.sin_port));
        ticks = time(NULL);
        snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
        write(connfd, buf, strlen(buf));
        close(connfd);
    }

    return 0;
}
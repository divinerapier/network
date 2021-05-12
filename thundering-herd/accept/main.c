#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SERVER_ADDRESS "0.0.0.0"
#define SERVER_PORT 10086
#define WORKER_COUNT 4

int worker_process(int listenfd, int i) {
    while (1) {
        printf("I am work %d, my pid is %d, begin to accept connections \n", i,
               getpid());
        struct sockaddr_in client_info;
        socklen_t client_info_len = sizeof(client_info);
        int connection =
            accept(listenfd, (struct sockaddr *)&client_info, &client_info_len);
        if (connection != -1) {
            printf("worker %d accept success\n", i);
            printf("ip: %s\t", inet_ntoa(client_info.sin_addr));
            printf("port: %d \n", htons( client_info.sin_port));
            char buf[1024] = {0};
            sprintf(buf, "\nhello, %s:%d\n\n", inet_ntoa(client_info.sin_addr), htons(client_info.sin_port));
            write(connection, buf, strlen(buf) +1);
        } else {
            printf("worker %d accept failed", i);
        }
        close(connection);
    }

    return 0;
}

int main() {
    int i = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_ADDRESS, &address.sin_addr);
    address.sin_port = htons(SERVER_PORT);
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    int ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    if (ret != 0 ) {
        perror("bind error");
        return 0;
    }
    ret = listen(listenfd, 5);
    if (ret != 0 ) {
        perror("listen error");
        return 0;
    }
    for (i = 0; i < WORKER_COUNT; i++) {
        printf("Create worker %d\n", i + 1);
        pid_t pid = fork();
        /*child  process */
        if (pid == 0) {
            worker_process(listenfd, i);
        }
        if (pid < 0) {
            printf("fork error");
        }
    }

    /*wait child process*/
    int status;
    wait(&status);
    return 0;
}
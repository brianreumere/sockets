#define MAX_BACKLOG 10
#define MAX_LENGTH 1024

#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void run_client(int socketfd) {
    char recv_buffer[MAX_LENGTH], line_buffer[MAX_LENGTH];
    char *line_pointer = NULL;
    size_t line_size = 0, read_size = 0;
    ssize_t num_chars = 0, recv_size = 0, send_size = 0;
    fd_set read_fds;
    struct timeval;
    int max_fd = FD_SETSIZE + 1, stdin_eof = 0;
    FD_ZERO(&read_fds);
    printf(">>> ");
    fflush(stdout);
    for (;;) {
        /* select */
        FD_SET(socketfd, &read_fds);
        if (stdin_eof == 0) {
            FD_SET(fileno(stdin), &read_fds);
        }
        if (select(max_fd, &read_fds, NULL, NULL, NULL) == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                err(1, "select");
            }
        }
        if (FD_ISSET(socketfd, &read_fds)) { /* socket is readable */
            bzero(recv_buffer, sizeof(recv_buffer));
            /* recv */
            recv_size = recv(socketfd, recv_buffer, sizeof(recv_buffer), 0);
            if (recv_size == -1) {
                err(1, "recv");
            } else if (recv_size == 0) {
                if (stdin_eof == 1) {
                    return; /* normal */
                } else {
                    err(1, "recv"); /* server process terminated */
                }
            }
            printf("%s", recv_buffer);
            printf(">>> ");
            fflush(stdout);
        }
        if (FD_ISSET(fileno(stdin), &read_fds)) { /* stdin is readable */
            bzero(line_buffer, sizeof(line_buffer));
            /*line_pointer = NULL;
            getline(&line_pointer, &line_size, stdin);
            strlcpy(line_buffer, line_pointer, sizeof(line_buffer));*/
            read_size = read(fileno(stdin), line_buffer, sizeof(line_buffer));
            if (read_size == 0) { /* Detect EOF if using batch input */
                stdin_eof = 1;
                shutdown(socketfd, SHUT_WR); /* send FIN */
                FD_CLR(fileno(stdin), &read_fds);
                continue;
            } else if (read_size == -1) {
                err(1, "read");
            }
            /* send */
            if ((send_size = send(socketfd, line_buffer, strlen(line_buffer), 0)) == -1) {
                err(1, "send");
            }
            /*free(line_pointer);*/
        }
    }
}

int main(int argc, char *argv[]) {
    int server_port, socket_descriptor, connect_status;
    struct sockaddr_in server_address;
    if (argv[1] == NULL) {
        err(1, "argv");
    }
    server_port = strtol(argv[1], NULL, 10);
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    /* socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        err(1, "socket");
    }
    /* connect */
    if ((connect_status = connect(socket_descriptor, (struct sockaddr *) &server_address, sizeof(server_address))) != 0) {
        err(1, "connect");
    }
    run_client(socket_descriptor);
    close(socket_descriptor);
}

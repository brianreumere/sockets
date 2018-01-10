#define MAX_BACKLOG 10
#define MAX_LENGTH 1024

#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void cleanup_zombies(int sig) {
    pid_t child_pid;
    int status;
    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) {
        /*
         * Not safe to call printf() from a signal handler, this is just for
         * debugging.
        */
        printf("terminated child with pid %d\n", child_pid);
    }
}

void handle_client(int acceptfd) {
    ssize_t recv_size, send_size;
    char recv_buffer[MAX_LENGTH];
    while (1) {
        bzero(recv_buffer, sizeof(recv_buffer));
        /* recv */
        recv_size = recv(acceptfd, recv_buffer, sizeof(recv_buffer), 0);
        if (recv_size > 0) {
            /* send */
            if ((send_size = send(acceptfd, recv_buffer, strlen(recv_buffer), 0)) == -1) {
                err(1, "send");
            }
        } else if (recv_size == 0) {
            _exit(0);
        } else {
            err(1, "recv");
        }
    }
}

int main(int argc, char *argv[]) {
    int server_port, socket_descriptor, bind_status, listen_status, accept_descriptor;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    struct sigaction sigchld_act;
    struct sigaction sigint_act;
    socklen_t client_address_length;
    pid_t child_pid;
    if (argv[1] == NULL) {
        err(1, "argv");
    }
    /* sockaddr_in */
    server_port = strtol(argv[1], NULL, 10);
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    /* socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        err(1, "socket");
    }
    /* bind */
    if ((bind_status = bind(socket_descriptor, (struct sockaddr *) &server_address, sizeof(server_address))) == -1) {
        err(1, "bind");
    }
    /* listen */
    if ((listen_status = listen(socket_descriptor, MAX_BACKLOG)) == -1) {
        err(1, "listen");
    }
    /* sigaction */
    bzero(&sigchld_act, sizeof(sigchld_act));
    sigchld_act.sa_handler = cleanup_zombies;
    sigchld_act.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sigchld_act, NULL) == -1) {
        err(1, "sigaction");
    }
    /* select */
    /*fd_set read_fds, write_fds, except_fds;
    struct timeval; 
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);*/
        /* check for EINTR */
    for (;;) {
        bzero(&client_address, sizeof(client_address));
        /* accept */
        if ((accept_descriptor = accept(socket_descriptor, (struct sockaddr *) &client_address, &client_address_length)) == -1) {
            /* restart accept() if interrupted by a signal */
            /* accept() does *not* restart even with SA_RESTART flag, see sigaction(2) man page */
            if (errno == EINTR) {
                continue;
            } else {
                err(1, "accept");
            }
        }
        /* fork */
        if ((child_pid = fork()) == 0) {
            close(socket_descriptor);
            handle_client(accept_descriptor);
        }
        close(accept_descriptor);
    }
}

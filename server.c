#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int sid;
	int port;
    int bstatus;
    int lstatus;
    int csid;
    struct sockaddr_in iaddr;
    struct sockaddr_in caddr;
    pid_t procid;
    socklen_t csize = sizeof(caddr);
    ssize_t rsize;
    ssize_t ssize;
    char rbuf[1024];
    sid = socket(AF_INET, SOCK_STREAM, 0);
    if (sid == -1) {
        err(1, "socket");
    }
    if (argv[1] == NULL) {
        err(1, "argv");
    }
    port = strtol(argv[1], NULL, 10);
    memset(&iaddr, 0, sizeof(iaddr));
    iaddr.sin_family = AF_INET;
    iaddr.sin_addr.s_addr = INADDR_ANY;
    iaddr.sin_port = htons(port);
    iaddr.sin_len = sizeof(iaddr);
    bstatus = bind(sid, (struct sockaddr *) &iaddr, sizeof(iaddr));
    if (bstatus == -1) {
        err(1, "bind");
    }
    lstatus = listen(sid, 2);
    if (lstatus == -1) {
        err(1, "listen");
    }
    while (1) {
        memset(&caddr, 0, sizeof(caddr));
        csid = accept(sid, (struct sockaddr *) &caddr, &csize);
        if (csid == -1) {
            err(1, "accept");
        }
        procid = fork();
        if (procid == 0) {
            close(sid);
            while (1) {
                bzero(rbuf, 1024);
                rsize = recv(csid, rbuf, sizeof(rbuf), 0);
                if (rsize == -1) {
                    err(1, "recv");
                } else if (rsize == 0) {
                    printf("client exited\n");
                    _exit(0);
                }
                rbuf[strcspn(rbuf, "\n")] = '\0';
                ssize = send(csid, rbuf, sizeof(rbuf), 0);
            }
        } else {
            printf("waitpid...\n");
            waitpid((pid_t) procid, NULL, WNOHANG);
        }
    }
    close(csid);
    close(sid);
}

#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    int sid;
	int port;
    int sstatus;
    struct sockaddr_in saddr;
    socklen_t ssize = sizeof(saddr);
    char msg[] = "command";
    ssize_t ll;
    char *lp = NULL;
    size_t ls = 0;
    char buf[1024];
    char rbuf[1024];
    sid = socket(AF_INET, SOCK_STREAM, 0);
    if (sid == -1) {
        err(1, "socket");
    }
    port = strtol(argv[1], NULL, 10);
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddr.sin_port = htons(port);
    sstatus = connect(sid, (struct sockaddr *) &saddr, ssize);
    if (sstatus == -1) {
        err(1, "connect");
    }
    while (1) {
        printf("> ");
        getline(&lp, &ls, stdin);
        strlcpy(buf, lp, sizeof(buf));
        //buf[strcspn(buf, "\n")] = '\0';
        send(sid, buf, strlen(buf), 0);
        recv(sid, rbuf, 1024, 0);
        printf("%s\n", rbuf);
    }
    free(lp);
    close(sid);
}

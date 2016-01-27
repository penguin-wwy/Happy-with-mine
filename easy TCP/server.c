#include "unp.h"

void str_echo(int);
ssize_t writen(int fd, const void *vptr, size_t n);
void sig_child(int signo);

int main(int argc, char **argv)
{
    int     listenfd, connfd;
    pid_t   childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    void    sig_child(int);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    Signal(SIGCHLD, sig_child);

    for(;;){
        clilen = sizeof(cliaddr);
        printf("main wait for receiving\n");
        if((connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0){
            if(EINTR == errno)
                continue;
            else
                printf("accept error\n");
        }

        printf("child is staring\n");
        if((childpid = fork()) == 0){
            close(listenfd);
            str_echo(connfd);
            exit(0);
        }
    close(connfd);
    }
}

void str_echo(int sockfd)
{
    ssize_t n;
    char    buf[MAXLINE];

    printf("child start to recvive\n");
again:
    while((n = read(sockfd, buf, MAXLINE)) > 0){
        writen(sockfd, buf, n);
    }

    if(n < 0 && errno == EINTR)
        goto again;
    else if (n < 0)
        printf("str_echo: read error");
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while(nleft > 0){
        if((nwritten = write(fd, ptr, nleft)) <= 0){
            if(nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                return 0;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

Sigfunc *Signal(int signo, Sigfunc *func)
{
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if(SIGALRM == signo){
#ifdef  SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;
#endif // SA_INTERRUPT
    }else{
#ifdef  SA_RESTART
    act.sa_flags |= SA_RESTART;
#endif // SA_RESTART
    }
    if(sigaction(signo, &act, &oact) < 0)
        return SIG_ERR;
    return oact.sa_handler;
}

void sig_child(int signo)
{
    pid_t   pid;
    int     stat;

    while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
    //pid = wait(&stat);
        printf("child %d terminated\n", pid);
    return;
}

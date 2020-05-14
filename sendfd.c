#include <stdio.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main()
{
    int ret = -1;
    struct iovec iov;
    struct msghdr msg;
    struct sockaddr_un caddr;
    int msg_len = CMSG_SPACE(sizeof(1));
    char cmsgbuf[msg_len];
    
    int fd = open("t1.txt", O_WRONLY);
    if(fd < 0) {
        fprintf(stdout, "file open error");
        return -1;
    }

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd < 0) {
        fprintf(stdout, "socket open error");
        goto cfd;
    }
    
    memset(&caddr, 0, sizeof(caddr));
    caddr.sun_family = AF_UNIX;
//    memcpy(caddr.sun_path, "sendfd.sock", 11);

    while(connect(sockfd, (struct sockaddr *)&caddr,
                sizeof(short)+1/*sizeof(caddr)*/) < 0) {
        
        if(errno == EINPROGRESS || errno == EINTR)
            continue;

        fprintf(stdout, "bind error");
        goto csk;
    }

    memset(cmsgbuf, 0, msg_len);
    struct cmsghdr *cmsg = (struct cmsghdr*)cmsgbuf;
    cmsg->cmsg_level = SOCK_STREAM;
    cmsg->cmsg_len = msg_len;
    cmsg->cmsg_type = SCM_RIGHTS;
    *(int *)CMSG_DATA(cmsg) = fd;

    iov.iov_base = "1";
    iov.iov_len = 1;

    msg.msg_control = cmsgbuf;
    msg.msg_controllen = cmsg->cmsg_len;
    msg.msg_flags = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_namelen = 0;
    msg.msg_name = NULL;

    ret = sendmsg(sockfd, &msg, 0);
    if(ret <= 0) 
        fprintf(stdout, "sendmsg error %d: %s\n", ret, strerror(errno));
    
    ret = 0;

csk:
    close(fd);
cfd:
    close(sockfd);
    return ret;
}

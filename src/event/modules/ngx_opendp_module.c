#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <netinet/in.h>
#include <termios.h>
#include <sys/epoll.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifndef __linux__
#ifdef __FreeBSD__
#include <sys/socket.h>
#else
#include <net/socket.h>
#endif
#endif

#include <sys/time.h>
#include "netdpsock_intf.h"
#include "netdp_errno.h"

#define _GNU_SOURCE
#define __USE_GNU
#ifdef __USE_GNU
/* Access macros for `cpu_set'.  */
#define CPU_SETSIZE __CPU_SETSIZE
#define CPU_SET(cpu, cpusetp)   __CPU_SET (cpu, cpusetp)
#define CPU_CLR(cpu, cpusetp)   __CPU_CLR (cpu, cpusetp)
#define CPU_ISSET(cpu, cpusetp) __CPU_ISSET (cpu, cpusetp)
#define CPU_ZERO(cpusetp)       __CPU_ZERO (cpusetp)
#endif
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <dlfcn.h>

#define ODP_FD_BITS 30


static int inited = 0;
void ngx_opendp_init()
{
    int rc;
    rc = netdpsock_init(NULL);
    assert(0 == rc);
    inited = 1;
}

int socket(int domain, int type, int protocol)
{
    int rc;
    static int (*real_socket)(int, int, int) = NULL;
    
    if (!real_socket) {
        real_socket = dlsym(RTLD_NEXT, "socket");
    }

    if (AF_INET != domain || SOCK_STREAM != type) {
        return real_socket(domain, type, protocol);
    }

    assert(inited);
    rc = netdpsock_socket(domain, type, protocol);
    if (rc >= 0) {
        rc |= 1 << ODP_FD_BITS;
    }
        
    return rc;
}

int socketpair (int __domain, int __type, int __protocol,
               int __fds[2])
{
    return -1;
}

int bind (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len)
{
    return -1;
}

int getsockname (int __fd, __SOCKADDR_ARG __addr,
            socklen_t *__restrict __len)
{
    return -1;
}

int connect (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len)
{
    return -1;
}

int getpeername (int __fd, __SOCKADDR_ARG __addr,
            socklen_t *__restrict __len)
{
    return -1;
}

ssize_t send (int __fd, const void *__buf, size_t __n, int __flags)
{
    return -1;
}

ssize_t recv (int __fd, void *__buf, size_t __n, int __flags)
{
    return -1;
}

ssize_t sendto (int __fd, const void *__buf, size_t __n,
               int __flags, __CONST_SOCKADDR_ARG __addr,
                              socklen_t __addr_len)
{
    return -1;
}

ssize_t recvfrom (int __fd, void *__restrict __buf, size_t __n,
             int __flags, __SOCKADDR_ARG __addr,
             socklen_t *__restrict __addr_len)
{
    return -1;
}

ssize_t sendmsg (int __fd, const struct msghdr *__message,
            int __flags)
{
    return -1;
}

ssize_t recvmsg (int __fd, struct msghdr *__message, int __flags)
{
    return -1;
}

int getsockopt (int __fd, int __level, int __optname,
               void *__restrict __optval,
               socklen_t *__restrict __optlen)
{
    return -1;
}

int setsockopt (int __fd, int __level, int __optname,
               const void *__optval, socklen_t __optlen)
{
    return -1;
}

int listen (int __fd, int __n)
{
    return -1;
}

int accept (int __fd, __SOCKADDR_ARG __addr,
               socklen_t *__restrict __addr_len)
{
    return -1;
}

int accept4(int sockfd, struct sockaddr *addr,
               socklen_t *addrlen, int flags)
{
    return -1;
}

int shutdown (int __fd, int __how)
{
    return -1;
}

int close(int fd)
{
    static int (*real_close)(int) = NULL;

    if (!real_close) {
        real_close = dlsym(RTLD_NEXT, "close");
    }

    if (fd & (1 << ODP_FD_BITS)) {
        fd &= ~(1 << ODP_FD_BITS);
        return netdpsock_close(fd);
    } else {
        return real_close(fd);
    }
}

int epoll_create (int __size)
{
    return -1;
}

int epoll_create1 (int __flags)
{
    return -1;
}

int epoll_ctl (int __epfd, int __op, int __fd,
              struct epoll_event *__event)
{
    return -1;
}

int epoll_wait (int __epfd, struct epoll_event *__events,
               int __maxevents, int __timeout)
{
    return -1;
}

int epoll_pwait (int __epfd, struct epoll_event *__events,
            int __maxevents, int __timeout,
            const __sigset_t *__ss)
{
    return -1;
}


// fcntl
// ioctl
// read
// write
// sendfile



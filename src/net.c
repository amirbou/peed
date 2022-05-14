#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <limits.h>

#include "net.h"
#include "log.h"



static int _get_addr(int (*func)(int, struct sockaddr *, socklen_t *), int sockfd, struct sockaddr_in * addr)
{
    socklen_t addrlen = sizeof(*addr);

    if (-1 == func(sockfd, (struct sockaddr *)addr, &addrlen))
    {
        PERROR("getpeername or getsockname");
        return -1;
    }

    if (addrlen != sizeof(*addr))
    {
        ERROR("addrlen mismatch, address is not ipv4, addrlen: %u", addrlen);
        return -1;
    }

    if (addr->sin_family != AF_INET)
    {
        ERROR("address family mismatch, address is not AF_INET, sin_family: %d", addr->sin_family);
        return -1;
    }

    return 0;
    
}
static int get_server_addr(int sockfd, struct sockaddr_in * server_addr)
{
    return _get_addr(getsockname, sockfd, server_addr);
}

static int get_peer_addr(int sockfd, struct sockaddr_in * peer_addr)
{
    return _get_addr(getpeername, sockfd, peer_addr);
}


/*
    Parse /proc/net/tcp and search for the inode of the socket connecting from peer_addr to server_addr
*/
static ino_t get_socket_inode(struct sockaddr_in * server_addr, struct sockaddr_in * peer_addr) {
    const char proc_tcp[] = "/proc/net/tcp";
    const char scan_fmt[] = " %*d: %8X:%4X %8X:%4X %*X %*X:%*X %*X:%*X %*X %*d %*d %lu %*s\n";    
    const uint32_t hserver_ip = server_addr->sin_addr.s_addr;
    const uint32_t hpeer_ip = peer_addr->sin_addr.s_addr;
    const uint16_t hserver_port = ntohs(server_addr->sin_port);
    const uint16_t hpeer_port = ntohs(peer_addr->sin_port);

    DEBUG("looking for laddr: %X %d raddr: %X %d", hpeer_ip, hpeer_port, hserver_ip, hserver_port);
    ino_t result = 0;

    FILE * tcp_file = fopen(proc_tcp, "r");
    if (tcp_file == NULL)
    {
        PERROR("fopen /proc/net/tcp");
        return 0;
    }

    // netstat uses a buffer with this size, so we should be good.
    char linebuf[8192] = { 0 };
    if (NULL == fgets(linebuf, sizeof(linebuf), tcp_file))
    {
        PERROR("fgets first line of /proc/net/tcp");
        goto cleanup;
    }

    while (NULL != fgets(linebuf, sizeof(linebuf), tcp_file))
    {
        uint32_t local_addr = 0xffffffff;
        uint32_t remote_addr = 0xffffffff;
        uint16_t local_port = 0;
        uint16_t remote_port = 0;
        uint32_t llocal_port = 0;
        uint32_t lremote_port = 0;
        ino_t inode = 0;
        int num = sscanf(linebuf, scan_fmt, &local_addr, &llocal_port, &remote_addr, &lremote_port, &inode);
        if (5 != num)
        {
            ERROR("failed to scan line %s, ret: %d", linebuf, num);
            goto cleanup;
        }
        local_port = (uint16_t)llocal_port;
        remote_port = (uint16_t)lremote_port;
        DEBUG("tcp line parsed: laddr: %X %d raddr: %X %d inode: %lu", local_addr, local_port, remote_addr, remote_port, inode);
        if ((local_addr == hpeer_ip) && (remote_addr == hserver_ip) && \
            (local_port == hpeer_port) && (remote_port == hserver_port))
        {
            result = inode;
            break;
        }
    }


cleanup:
    if (tcp_file != NULL)
    {
        fclose(tcp_file);
    }
    return result;
}

ino_t get_client_inode(int sockfd)
{
    struct sockaddr_in peer_addr = { 0 };
    struct sockaddr_in server_addr = { 0 };
    if (-1 == get_peer_addr(sockfd, &peer_addr))
    {
        INFO("Failed to get peer addr");
        return 0;
    }
    if (-1 == get_server_addr(sockfd, &server_addr))
    {
        INFO("Failed to get server addr");
        return 0;
    }

    DEBUG("peer: %s %u", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

    if (peer_addr.sin_addr.s_addr == INADDR_LOOPBACK)
    {
        INFO("peer is not from localhost");
        return 0;
    }

    ino_t inode = get_socket_inode(&server_addr, &peer_addr);
    if (0 == inode)
    {
        INFO("failed to find socket inode");   
    }
    return inode;
}
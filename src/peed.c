#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <limits.h>

#include "peed.h"
#include "net.h"
#include "proc.h"
#include "log.h"

pid_t find_peer_pid(int sockfd) {
    int result = -1;

    ino_t inode = get_client_inode(sockfd);
    if (inode == 0)
    {
        return -1;
    }
    DEBUG("inode: %lu\n", inode);

    return get_socket_inode_pid(inode);
    
}
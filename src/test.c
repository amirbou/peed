#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>

#include "peed.h"

#define PORT (6666)


int main()
{
    int server = socket(AF_INET, SOCK_STREAM, 0);
    int client = -1;
    int true = 1;
    int result = EXIT_FAILURE;
    if (-1 == server)
    {
        perror("socket");
        return -1;
    }

    const struct sockaddr_in addr = { .sin_family=AF_INET, .sin_port=htons(PORT), .sin_addr=htonl(INADDR_ANY)};

    if (-1 == setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(true)))
    {
        perror("setsockopt reuseaddr");
        goto cleanup;
    }
    if (-1 == bind(server, (const struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind");
        goto cleanup;
    }

    if (-1 == listen(server, 1))
    {
        perror("listen");
        goto cleanup;
    }

    client = accept(server, NULL, NULL);
    if (-1 == client)
    {
        perror("accept");
        goto cleanup;
    }

    pid_t pid = find_peer_pid(client);
    printf("pid %d\n", pid);

    pause();
    result = EXIT_SUCCESS;
cleanup:
    if (-1 != client) close(client);
    if (-1 != server) close(server);
    return result;

}
/*
    Finds the inode of the socket of the client connected to sockfd (will fail if the client is not on the same machine)
    Returns 0 on failure.
*/
ino_t get_client_inode(int sockfd);

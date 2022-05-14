#include <sys/types.h>

/*
Finds the pid of the process that opened the socket described by socket_inode
*/
pid_t get_socket_inode_pid(ino_t socket_inode);
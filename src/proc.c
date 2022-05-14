#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdbool.h>
#include <dirent.h>

#include "proc.h"
#include "log.h"

static int search_process(const char * process_pid, const char * socket_inode)
{
    bool result = false;
    char pathbuf[PATH_MAX] = { 0 };
    // this is the size used by netstat.
    char link_name[30] = { 0 };
    snprintf(pathbuf, sizeof(pathbuf), "/proc/%s/fd/", process_pid);
    char * pathbuf_tail = pathbuf + strlen(pathbuf);

    struct dirent * entry = NULL;

    DIR * fddir = opendir(pathbuf);
    if (NULL == fddir)
    {
        return false;
    }

    while(NULL != (entry = readdir(fddir)))
    {
        // skip . and ..
        if (!isdigit(entry->d_name[0]))
        {
            continue;
        }
        strcpy(pathbuf_tail, entry->d_name);
        ssize_t link_size = readlink(pathbuf, link_name, sizeof(link_name) - 1);
        if (-1 == link_size)
        {
            // PERROR("readlink");
            continue;
        }
        link_name[link_size] = '\0';
        if (0 == strcmp(link_name, socket_inode))
        {
            // bingo!
            result = true;
            break;
        }

    }

cleanup:
    if (NULL != fddir)
    {
        closedir(fddir);
    }
    return result;
}

pid_t get_socket_inode_pid(ino_t socket_inode)
{
    pid_t result = -1;
    char socket_symlink[1024] = { 0 };
    snprintf(socket_symlink, sizeof(socket_symlink), "socket:[%lu]", socket_inode);

    struct dirent * entry = NULL;

    DIR * procdir = opendir("/proc/");
    if (NULL == procdir)
    {
        PERROR("opendir /proc");
        return -1;
    }

    while (NULL != (entry = readdir(procdir)))
    {
        // low effort test to not waste time on non pid values
        if (!isdigit(entry->d_name[0]))
        {
            continue;
        }
        if (search_process(entry->d_name, socket_symlink))
        {
            // if we found the socket there it must be a valid pid
            result = (pid_t)atoll(entry->d_name);
            break;
        }
    }
    
cleanup:
    if (procdir != NULL)
    {
        closedir(procdir);
    }
    return result;
}
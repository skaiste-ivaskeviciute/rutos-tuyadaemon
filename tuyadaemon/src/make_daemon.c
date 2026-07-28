#include "make_daemon.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

int make_daemon() {
    int maxfd, fd;

    switch (fork()) {
    case -1: 
        exit(EXIT_FAILURE);
    case 0: 
        break; // Child falls through
    default: 
        exit(EXIT_SUCCESS); // Parent terminates
    }

    if (setsid() == -1) // Become leader of new session
        exit(EXIT_FAILURE);

    switch (fork()) {
    case -1: 
        exit(EXIT_FAILURE);
    case 0: 
        break; // Child falls through
    default: 
        exit(EXIT_SUCCESS); // Parent terminates
    }

    // Clear file creation mode mask
    umask(0);                       
    // Change to root directory
    chdir("/"); 

    // Close all open files
    maxfd = sysconf(_SC_OPEN_MAX);
    if (maxfd == -1)     
        maxfd = 8192; 
    for (fd = 0; fd < maxfd; fd++) {
            close(fd);
    }
    
    // Close stdin, point stdout and stderr to /dev/null
    close(STDIN_FILENO);

    fd = open("/dev/null", O_RDWR);
    if (fd != STDIN_FILENO)
      return -1;
    if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
      return -2;
    if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
      return -3;

    return 0;
}
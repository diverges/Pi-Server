/*
 
server.c: stream socket echo server.

*/

#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT "2525"

// Functions
void *get_in_addr(struct sockaddr *sa);

int main()
{
    // Listener and new connection handler
    int sockfd, new_fd; 
    struct addrinfo hints, *servinfo, *p;

    // connectors information
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    int rv, err_fd;
    int yes=1;

    // generate address hint
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // this ip

    if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        close(err_fd);
        return 1;
    }

    // Setup and bind a socket
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        // Attempt to open socket
        if((sockfd = socket(p->ai_family, p->ai_socktype, 
            p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        // Socket options
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, 
            sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        // Bind socket
        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if(p == NULL) 
    {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }
    
    freeaddrinfo(servinfo);

    // Listen on socket
    if(listen(sockfd, 2) == -1)
    {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections... \n");

    // main accept() loop
    while(1)
    {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if(new_fd == -1)
        {
            perror("server: accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, 
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        /* Handle connection Event */
        if (send(new_fd, "Hello!\n", 7, 0) == -1) perror("send");
        close(new_fd);
        break;
    }
    close(sockfd);
    return 0;
}

// Get socket address in IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET) 
        return &(((struct sockaddr_in*)sa)->sin_addr);

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


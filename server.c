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
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

#define PORT        "2525"
#define BACKLOG     5
#define BUFFER_SIZE 128

// Functions
void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
void run(char* s, int new_fd);

int main()
{
    // Listener and new connection handler
    int sockfd, new_fd; 
    struct addrinfo hints, *servinfo, *p;

    // connectors information
    struct sigaction sa;
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

    printf("Initializing server...\n");

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

        printf("Binding socket...\n");

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
    if(listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
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

        /* Handle connection Event */
        if(!fork())
        {
            /* Child Process */
            close(sockfd); // child does not need
            run(s, new_fd); // should not return                      
        }

        close(new_fd); // parent does not need this
    }
    close(sockfd);
    return 0;
}

// run(char* s)
// s: string representing remote ip
// must close child socket, should not return
void run(char* s, int new_fd)
{
    int n;
    int cont;
    char* parse;
    char buf[BUFFER_SIZE];

    printf("server: got connection from %s\n", s);
    if (send(new_fd, "Welcome! Type 'exit' to terminate connection.\n", 46, 0) == -1) perror("send");
    
    /* Read Commands */
    cont = 1;
    while(cont != 0)
    {
        // clear buff
        memset(&buf, 0, sizeof buf);
        if((n = read(new_fd, buf, BUFFER_SIZE)) < 0) perror("read"); 
        
        parse = strtok(buf, " ");
        if(parse != NULL)
            printf("server: recieved command %s", buf);
        
        /* Translate Command */
        //
        // single word instructions must end in \r\n, see exit 
        if(strcmp(parse, "exit\r\n") == 0) 
        {        
            printf("server: closing connection with %s\n", s);
            cont = 0;
        }   
        else if (strcmp(parse, "echo") == 0)
        {
            parse = strtok(NULL, "");
            printf("server: sending (%s): %s\n", s, parse);
            if((n=write(new_fd, parse, strlen(parse))) < 0) perror("write");
        }
        else if (strcmp(parse, "exec\r\n") == 0)
        {
            printf("server: executing args.py\n");
            if(!fork())
            {
                if(execlp("python", "python", "py/args.py", NULL, NULL) < 0)
                {
                    perror("execlp");
                    exit(0);
                }
            }
        }
        else
        {
            printf("server: unknown command.\n");   
        }
    }

    close(new_fd);
    exit(0);
}

// Get socket address in IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET) 
        return &(((struct sockaddr_in*)sa)->sin_addr);

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Reap zombies
void sigchld_handler(int s)
{
    printf("server: sigchld called\n");
    while(waitpid(-1, NULL, WNOHANG) > 0);
}


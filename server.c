/*
 *  Pi Server:
 *    
 *  Basic server designed to manage python application
 *  on a Raspberry Pi (not limited to). Meant to allow
 *  client applications on same network to initiate 
 *  various programs.
 *
 */

#include <arpa/inet.h>
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
#include <pthread.h>
#include "jobs.h"


#define PORT            "2525"
#define BACKLOG         5
#define BUFFER_SIZE     1024
#define MAX_ARGUMENTS   10

// Thread Argument Struct
struct run_args 
{
    char  command[INET6_ADDRSTRLEN];    /* client request */
    int   new_fd;                       /* client socket  */
};

// Job List
struct job_t job_list[MAXJOBS];

// Functions
void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
void *run(void* args);
int  swrite(int fd, char*buf, int length);  

int main()
{
    // Listener and new connection handler
    int sockfd, new_fd; 
    struct addrinfo hints, *servinfo, *p;

    // connectors information
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    int rv;
    int yes=1;
    
    struct sigaction sa;
    sigset_t full_mask;

    // generate address hint
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // this ip

    if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
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

    inet_ntop(AF_INET, 
                p->ai_addr,
                s, sizeof s);

    //To-Do FIX
    printf("server: (INCORRECT) address %s\n", s);
    
    freeaddrinfo(servinfo);

    // Listen on socket
    if(listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }
    
    // sigchld handler
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // Block all signals
    sigfillset(&full_mask);
    pthread_sigmask(SIG_SETMASK, &full_mask, NULL);
    
    // job list
    printf("server: preparing job list...\n");
    initjobs(job_list);

    printf("server: waiting for connections... \n");

    // main accept() loop
    while(1)
    {
        pthread_t thread_id;
        struct run_args* r_args;

        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if(new_fd < 0)
        {
            perror("server: accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, 
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        
        /* Prepare Arguments */
        printf("Allocating client space... \n");
        // thread must free this
        r_args = (struct run_args*) malloc(sizeof(struct run_args));
        printf("Copying arguments...\n");
        strcpy(r_args->command, s);
        r_args->new_fd = new_fd;

        printf("Splitting thread...\n");
        
        // Creates a new thread to handle connection event 
        if(pthread_create(&thread_id, NULL, run, r_args) < 0)
        {
            perror("could not create thread");
            exit(1);
        }
        if(pthread_detach(thread_id) < 0)
        {
            perror("detach error");
            exit(1);
        }
        
    }

    close(sockfd);
    return 0;
}

// run(char* s)
// s: string representing remote ip
// must close child socket, should not return
void *run(void* args)
{
    // load arguments
    char* s = ((struct run_args*) args)->command;
    int new_fd = (((struct run_args*) args)->new_fd);

    int n;
    int cont;
    char* parse;
    char buf[BUFFER_SIZE];
    
    pid_t pid;
    sigset_t unblock_mask;

    int argc; // argument count (#args + NULL + 1)
    char* argv[MAX_ARGUMENTS]; // arguments
    
    // Initialize sigmasks
    if(sigemptyset(&unblock_mask) < 0) exit(1);
    if(sigaddset(&unblock_mask, SIGCHLD) < 0) exit(1);  
    if(sigaddset(&unblock_mask, SIGINT) < 0) exit(1);
    
    // Unblock Signals blocked by main thread
    if(pthread_sigmask(SIG_UNBLOCK, &unblock_mask, NULL) < 0)
        printf("server: FAILED sigblock\n");


    printf("server: got connection from %s on socket %u.\n", s, new_fd);
    if (swrite(
        new_fd, 
        "Welcome! Type 'exit' to terminate connection or 'shutdown' to close the server.\n", 
        80)) perror("send: bad");

    /* Read Commands */
    cont = 1;
    while(cont != 0)
    {
        // clear buff
        memset(buf, '\0', BUFFER_SIZE);
        
        // Read Input - TODO: Read until no input left?
        if((n = read(new_fd, buf, BUFFER_SIZE)) < 0) perror("read"); 
        
        // Parse first argument
        parse = strtok(buf, " ");
        if(parse != NULL)
        {
            printf("server: recieved command %s\n", buf);
        } 
        else
        {
            /* Read and parse failed */
            printf("Closing connection - freed %u\n", new_fd);
            close(new_fd);
            free(args);
            pthread_exit((void *) 0);
        }

        // Parse remainder of arguments
        argc = 1;
        argv[0] = parse;    
        while(parse != NULL)
        {
            parse = strtok(NULL, " ");
            argv[argc] = parse;
            argc++;
        }
        
        if(argc > MAX_ARGUMENTS)
        {
            printf("server: too many arguments - %u\n", argc);
            argv[0] = "echo";
            argv[1] = "error";
            argv[2] = NULL;
            argc = 3;
        }
        else
        {
            // Clean last argument, removes "\r\n" at end
            // Telnet adds those two
            argv[argc-2][strlen(argv[argc-2])-2] = '\0'; 
        }
        
        /**********************************************
         * Translate Command 
         **********************************************/
        if(strcmp(argv[0], "exit") == 0) 
        {        
            // Exit

            printf("server: closing connection with %s\n", s);
            cont = 0;
        }   
        else if (strcmp(argv[0], "echo") == 0)
        {
            // Echo
            printf("server: sending (%s)", s);
            for(n = 1; n < argc-1; n++)
            {
                if(swrite(new_fd, argv[n], strlen(argv[n])) || 
                   swrite(new_fd, " ", 1))    
                    perror("write");
            }
            if(swrite(new_fd, "\n", 1) < 0) perror("write");
        }
        else if ((strcmp(argv[0], "exec") == 0) && (argc > 1))
        {
            // Block SIGCHLD to prevent job_list race condition
            if(pthread_sigmask(SIG_BLOCK, &unblock_mask, NULL) < 0)
                printf("server: FAILED sigblock\n");

            //Exec - No pipe between parent and child
            if(!(pid = fork()))
            {
                // Child
                
                if(pthread_sigmask(SIG_UNBLOCK, &unblock_mask, NULL) < 0)
                    printf("server: FAILED sigblock\n");

                close(new_fd); // program won't need this
                free(args); // child won't need this
                if(execv("/bin/python", argv) < 0)
                {
                    perror("execlp");
                    exit(1);
                }
            }
            // Parent - Adds child to job list
            // assert: argc > 1
            if(addjob(job_list, pid, argv[1]) == 0) 
                printf("server: failed to add job.\n");
            if(pthread_sigmask(SIG_UNBLOCK, &unblock_mask, NULL) < 0)
                printf("server: FAILED sigblock\n");
        }
        else if(strcmp(argv[0], "jobs") == 0)
        {
            // Jobs
            // List running jobs
            // TODO: Currently only server side, needs to send
            //       list to client.
            printf("server: listing jobs\n");
            listjobs(job_list, buf, BUFFER_SIZE);
            swrite(new_fd, buf, strlen(buf));
        }
        else if((strcmp(argv[0], "kill") == 0) && (argc > 1))
        {
            // Kill <PID>
            // Send SIGINT to process with pid PID
            if((pid = atoi(argv[1])) == 0) perror("kill");
            printf("server: killing pid=%d\n", pid);
            if(pid) kill(pid, SIGINT);
        }
        else if(strcmp(argv[0], "shutdown") == 0)
        {
            // Shutdown
            // Tells the server to shutdown

            if(maxjid(job_list) != 0)
                printf("server: cannot shutdown, job(s) running\n");
            else
                exit(0);
        }
        else
        {
            // Unknown Command
            printf("server: unknown command.\n");   
        }
    }

    printf("Closing connection - freed %u\n", new_fd);
    close(new_fd);
    free(args);
    return 0;
}

// Get socket address in IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET) 
        return &(((struct sockaddr_in*)sa)->sin_addr);

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Sends a message to client
// fd: client fd
// buf: message to be sent
// length: characters to write
int  swrite(int fd, char* buf, int length)
{
    int n = 1;

    while((length > 0) && (n > 0))
    {
        n = send(fd, buf, length, 0);
        length -= n;
    }

    return (n < 0) ? 1 : 0;
}

// Reap zombies
// Parent handles job lsit from here
void sigchld_handler(int s)
{
    int status;
    pid_t pid;
    printf("server: sigchld called\n");
    
    while((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        printf("server removing job pid=%d\n", pid);
        // Delete job if terminated
        if(WIFEXITED(status) || WIFSIGNALED(status))
            deletejob(job_list, pid);
    }
}
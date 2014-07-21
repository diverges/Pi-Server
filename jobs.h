/*
 *  jobs.h
 * 
 *  Server Job Management 
 *
 *
 *
 */

#include <sys/types.h>
#include <string.h>
#include <stdio.h>

#define MAXJOBS 	10
#define MAXJDESC 	256

struct job_t
{
    pid_t pid;                  /* job PID */
    int jid;                    /* job ID [1, 2, ...] */
    char description[MAXJDESC]; /* job description */ 
};

/* Job list manipulation */
void listjobs(struct job_t *job_list, char* buf);
void clearjob (struct job_t *job);
void initjobs (struct job_t *job_list);
int maxjid (struct job_t *job_list);
int addjob (struct job_t *job_list, pid_t pid, char* description);
int deletejob (struct job_t *job_list, pid_t pid);
// int pid2jid (pid_t pid);
// struct job_t* getjobpid(struct job_t *job_list, pid_t pid);
// struct job_t* getjobjid(struct job_t *job_list, int jid);

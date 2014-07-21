/*
 *  jobs.c
 *
 *  Server Job Management 
 *
 */

#include "jobs.h"

/*************************************************
 * Globals and Test Main
 ************************************************/
int nextjid = 1;	// next JID to allocate

//#define DEBUG true

#ifdef DEBUG
struct job_t job_list[MAXJOBS];

int main()
{
	int i;
	char buf[MAXJDESC];

	initjobs(job_list);
	
	// Populate the job list
	for(i = 0; i < MAXJOBS; i++)
		addjob(job_list, i+1, "Job");

	// delete third job
	deletejob(job_list, 3);

	listjobs(job_list, buf);

    return 0;
}
#endif

/************************************************
 * Job List Functions - not thread safe 
 ***********************************************/

// listjobs
// Prints Job List to STDOUT
void listjobs(struct job_t *job_list, char* buf)
{
	int i;
	
	//if(buf != NULL);
	//	memset(buf, '\0', MAXJDESC);

	printf("-- Jobs [JID] (PID) \n");
	for(i = 0; i < MAXJOBS; i++)
	{
		if(job_list[i].pid != 0)
		{
			printf("\t[%d] (%d) %s\n", 
				job_list[i].jid, job_list[i].pid, job_list[i].description);
		}
	}
}

// clearjobs
// Clear a job list entry
void clearjob(struct job_t *job)
{
	job->pid = 0;
	job->jid = 0;
	job->description[0] = '\0';
}

// initjobs
// job_list: size MAXJOBS
// Prepares the job list by clearing all entries.
void initjobs(struct job_t *job_list)
{
	int i;

	for(i = 0; i < MAXJOBS; i++)
		clearjob(&job_list[i]);
} 

// maxjid
// returns: largest allocated JID, 0 if none
int maxjid(struct job_t *job_list)
{
	int i, max=0;

	for(i = 0; i < MAXJOBS; i++)
		if(job_list[i].jid > max)
			max = job_list[i].jid;
	return max;
}

// addjob
// Adds a job to the first available slot in job list
// returns 0 on failure
int addjob(struct job_t* job_list, pid_t pid, char* description)
{
	int i;

	if(pid < 1)
		return 0;
		
	for(i = 0; i < MAXJOBS; i++)
	{
		if(job_list[i].pid == 0)
		{
			printf("jlist: Adding Job [%d] (%d) %s\n", pid, nextjid,
				description);
			job_list[i].pid = pid;
			job_list[i].jid = nextjid++;
			strcpy(job_list[i].description, description);
			
			if(nextjid > MAXJOBS)
				nextjid = 1;
			
			return 1;
		}
	}

	printf("Tried to create too many jobs\n");
	return 0;
}

// deletejob
// Removes a job with unique PID from the job_list
// returns 0 on failure
int deletejob(struct job_t *job_list, pid_t pid)
{
	int i;

	if(pid < 1)
		return 0;
	
	for(i = 0; i < MAXJOBS; i++)
	{
		if(job_list[i].pid == pid)
		{
			printf("jlist: Removing Job [%d] (%d) %s\n", pid, 
				job_list[i].jid, job_list[i].description);
			clearjob(&job_list[i]);
			nextjid = maxjid(job_list)+1;
			return 1;
		}
	}

	return 0;
}

###############################################################################
#																			  #	
#	Pi Server																  #	
#	Author: Miguel Sotolongo (diverges)										  #
#																			  #	
###############################################################################

Last Modified:
	July 21, 2014

server: server.c 
	Listens on port #2525 for a connection, acts like a shell between the 
	rasberry pi and client application. Designed to execute and manage various
	python programs.
	Supported Commands:
		- exit: terminate client connection
		- shutdown: shutdown server
		- echo <message>: returns message to client
		- jobs: lists all currently running jobs server side
		- exec <py/file.py> [arg0] [arg1]...: executes file.py with arguments

jobs: jobs.c jobs.h
	Basic job management, server must initialize job_list for proper 
	functionality. 

Recently Completed:
	- Thread safe signal blocking (Completed: July 20, 2014)
	- Job Tracking, implement all of jobs.h (Completed July 20, 2014)
	- Proper exec, currently argv is being passed as a string rather than
	  being parsed server side. (Completed: July 08, 2014)
	- Threaded Implementation. (Completed: Junly 08, 2014)

To-Do:
	- Job Termination, let me send a kill command
	- Exec with pipe, allow client to send messages to programs it was
	  started.
	- Remote connection (not just local host)

Bugs (Features):
	- BUG: Running 'exec' with two consecutive spaces produces a '' 
	  argument.

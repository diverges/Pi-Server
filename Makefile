###############################################################################
#
#	Make file for Pi Server
#	Last Modified: July 23, 2014
#	
#	Type "make" to build server"
#
###############################################################################

CC = /usr/bin/gcc
CCFLAG = -Wall -g -Werror

server: server.c 
	$(CC) $(CCFLAG) -pthread -o pi_server server.c jobs.h jobs.c

jobs:
	gcc -o jobs jobs.c jobs.h
clean:
	rm -rf *.o *~ pi_server jobs


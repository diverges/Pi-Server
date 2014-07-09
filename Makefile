###############################################################################
#
#	Make file for Pi Server
#	Last Modified: July 7, 2014
#	
#	Type "make" to build server"
#
###############################################################################

CC = /usr/bin/gcc
CCFLAG = -Wall -g -Werror

server: server.c 
	$(CC) $(CCFLAG) -pthread -o server server.c

jobs:
	gcc -o jobs jobs.c jobs.h
clean:
	rm -rf *.o *~ server jobs


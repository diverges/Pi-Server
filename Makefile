server: server.c 
	gcc -o server server.c

clean:
	rm -rf *.o *~ server


all:
	gcc server.c -o server.out
	gcc client.c -o client.out
thread: 
	gcc server.c -DTHREADS  -pthread -o server.out
	gcc client.c -o client.out
process:
	gcc server.c -o server.out
	gcc client.c -o client.out
clean:
	rm -f server.out
	rm -f client.out

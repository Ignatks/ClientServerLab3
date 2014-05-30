#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define SERVER_PORT 20401
#define QUEUE_SIZE 5

void send_file(char* filename, int socket)
{
	int bytes_read;
	int bytes_written;
	char buffer[80];
	FILE* file = fopen(filename, "rb");

	// get file size
	fseek(file, 0L, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0L, SEEK_SET);

	// send file size
	write(socket, &size, sizeof(size_t));


	// send file
	do { 
	    bytes_read = fread(buffer, 1, sizeof(buffer), file);
    	bytes_written = write(socket, buffer, bytes_read);
	} while (bytes_read > 0);
	fclose(file);
}

void *thread_func(void *sock)
{
	int asock = *((int*)sock);
	char buffer[80];
	int error = recv(asock, buffer, sizeof(buffer), 0); // last parameter - flags

	// send file
	send_file(buffer, asock);
	close(asock);
}

void main (int argc, char *argv[])
{
	int error;
	int listen_sd;

	pthread_t pthread;
	int rc;
	
	int accept_sd;
	char buffer[80];
	struct sockaddr_in addr;
	pid_t child_pid;

	// create listening socket
	listen_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sd < 0) {
		perror("socket() failed");
		exit(-1);
	}

	// setoption error
	if (error < 0) {
		perror("setsockopt() failed");
		close(listen_sd);
		exit(-1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // converts a u_long from host to TCP/IP network byte order (which is big-endian).
	addr.sin_port  = htons(SERVER_PORT);
	error = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));

	if (error < 0) {
		perror("bind() failed");
		close(listen_sd);
		exit(-1);
   	}

	error = listen(listen_sd, QUEUE_SIZE);
	if (error < 0) {
		perror("listen() failed");
		close(listen_sd);
		exit(-1);
	}

	printf("The server is ready\n");

	while (1) {
		accept_sd = accept(listen_sd, NULL, NULL);
		if (accept_sd < 0) {
			perror("accept() failed");
			close(listen_sd);
			exit(-1);
		}
		
		#ifdef PTHREAD
		rc = pthread_create(&pthread, NULL, &thread_func, (void*)&accept_sd);
		if (rc) {
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
		#else
		pid_t childPid = fork();
		if (childPid == 0) {
			&thread_func((void*)&accept_sd);
			exit(0);
		}
		#endif
	}
	close(listen_sd);
}

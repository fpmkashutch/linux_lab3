#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <strings.h>

#define PORT 1337
#define BUFFER_SIZE 256

void process(int socketFileDescriptor) {
	char buffer[BUFFER_SIZE];
	FILE *file;
	int bytesRead;

	if (read(socketFileDescriptor, buffer, BUFFER_SIZE) < 0) {
		printf("ERROR, reading from socket.\n");
		return;
	}

	file = fopen(buffer, "rb");
	if (file == NULL) {
		printf("ERROR, opening file.\n");
		return;
	}

	while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
		if (write(socketFileDescriptor, buffer, bytesRead) < bytesRead) {
			printf("ERROR, writing to socket.\n");
			fclose(file);
			return;
		}
	if (bytesRead < 0) {
		printf("ERROR, reading file.\n");
		fclose(file);
		return;
	}

	fclose(file);
}

int createConnection(int portNumber) {
	int socketFileDescriptor, yes = 1;
	struct sockaddr_in serverAddress;

	if ((socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("ERROR, opening socket.\n");
		return -1;
	}

	bzero((char *) &serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (setsockopt(socketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(int)) < 0) {
		printf("ERROR, setting socket options.\n");
		close(socketFileDescriptor);
		return -1;
	}

	if (bind(socketFileDescriptor, (struct sockaddr *) &serverAddress,
			sizeof(serverAddress)) < 0) {
		printf("ERROR, binding.\n");
		close(socketFileDescriptor);
		return -1;
	}

	if (listen(socketFileDescriptor, 5) < 0) {
		printf("ERROR, listening.\n");
		close(socketFileDescriptor);
		return -1;
	}

	return socketFileDescriptor;
}

int main(int argc, char **argv) {
	int socketFileDescriptor;

	if ((socketFileDescriptor = createConnection(PORT)) < 0)
		return -1;

	while (1) {
		struct sockaddr_in clientAddress;
		pid_t pid;
		socklen_t clientAddressLength = sizeof(clientAddress);

		int newSocketFileDescriptor = accept(socketFileDescriptor,
				(struct sockaddr *) &clientAddress, &clientAddressLength);
		if (newSocketFileDescriptor < 0) {
			printf("ERROR, accepting.\n");
			break;
		}

		if ((pid = fork()) == 0) {
			process(newSocketFileDescriptor);
			close(newSocketFileDescriptor);
			break;
		} else {
			if (pid == -1)
				printf("ERROR, creating process.\n");
			close(newSocketFileDescriptor);
		}
	}
	close(socketFileDescriptor);
	return 0;
}

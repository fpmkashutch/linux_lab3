#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define PORT 1337
#define BUFFER_SIZE 256

int connectToServer(int portNumber) {
	int socketFileDescriptor;
	struct sockaddr_in serverAddress;
	struct hostent *server;

	if ((socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("ERROR, opening socket.\n");
		return -1;
	}

	if ((server = gethostbyname("localhost")) == NULL) {
		printf("ERROR, no such host.\n");
		close(socketFileDescriptor);
		return -1;
	}

	bzero((char *) &serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	bcopy((char *) server->h_addr,
	(char *)&serverAddress.sin_addr.s_addr,
	server->h_length);
	serverAddress.sin_port = htons(portNumber);

	if (connect(socketFileDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
		printf("ERROR, connecting to server.\n");
		close(socketFileDescriptor);
		return -1;
	}

	return socketFileDescriptor;
}

int downloadFile(int socketFileDescriptor, char fileName[BUFFER_SIZE]) {
	char buffer[BUFFER_SIZE];
	FILE *file = NULL;
	int bytesRead;

	if (write(socketFileDescriptor, fileName, BUFFER_SIZE) < 0) {
		printf("ERROR, writing to socket.\n");
		return -1;
	}

	while ((bytesRead = read(socketFileDescriptor, buffer, BUFFER_SIZE)) > 0) {
		if (file == NULL)
			if ((file = fopen(fileName, "wb")) == NULL) {
				printf("ERROR, opening file.\n");
				return -1;
			}
		if (fwrite(buffer, 1, bytesRead, file) < 0) {
			printf("ERROR, writing to file.\n");
			fclose(file);
			return -1;
		}
	}
	if (file != NULL)
		fclose(file);

	if (bytesRead <= 0) {
		printf("ERROR, downloading file.\n");
		return -1;
	}
	return 0;
}

int main(int argc, char **argv) {
	char fileName[BUFFER_SIZE];

	printf("Enter the file name: ");
	fgets(fileName, 255, stdin);

	if (strlen(fileName) > 1) {
		int socketFileDescriptor;

		fileName[strlen(fileName) - 1] = 0;

		if ((socketFileDescriptor = connectToServer(PORT)) < 0)
			return -1;
		if (downloadFile(socketFileDescriptor, fileName) == 0)
			printf("File was downloaded.\n");

		close(socketFileDescriptor);
	}
	return 0;
}

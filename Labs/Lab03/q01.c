#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc,char* argv[]) {

    int pipefd[2]; // File descriptors for the pipe
    char message[] = "Hello, Pipe!"; // Message to send through the pipe
    char buffer[256]; // Buffer to store the received message

    // Create the pipe
    if (pipe(pipefd) == -1) {
        perror("Pipe creation failed");
        return 1;
    }

    // Write the message to the pipe
    ssize_t bytesWritten = write(pipefd[1], message, strlen(message));
    if (bytesWritten == -1) {
        perror("Write failed");
        return 1;
    }

    printf("Sent message to pipe: %s\n", message);

    // Read the message from the pipe
    ssize_t bytesRead = read(pipefd[0], buffer, sizeof(buffer));
    if (bytesRead == -1) {
        perror("Read failed");
        return 1;
    }

    // Null-terminate the received message
    buffer[bytesRead] = '\0';

    // Display the received message
    printf("Received message from pipe: %s\n", buffer);

    // Close both ends of the pipe
    close(pipefd[0]);
    close(pipefd[1]);
    
    return 0;
}

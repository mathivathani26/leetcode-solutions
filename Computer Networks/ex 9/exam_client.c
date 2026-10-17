#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5000
#define MAX 1024

int main()
{
    int sock;
    struct sockaddr_in server;
    char buffer[MAX];
    char input[100];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        perror("socket");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server,
                sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    while (1) {
        int n = recv(sock, buffer, sizeof(buffer) - 1, 0);

        if (n <= 0)
            break;

        buffer[n] = '\0';
        printf("%s", buffer);
        fflush(stdout);

        if (strstr(buffer, "Username:") != NULL ||
            strstr(buffer, "Password:") != NULL ||
            strstr(buffer, "Enter option") != NULL) {

            if (fgets(input, sizeof(input), stdin) == NULL)
                break;

            send(sock, input, strlen(input), 0);
        }

        if (strstr(buffer, "Final score:") != NULL)
            break;

        if (strstr(buffer, "Invalid username or password") != NULL)
            break;
    }

    close(sock);
    return 0;
}

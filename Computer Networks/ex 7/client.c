#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5000

struct Message
{
    float num1;
    float num2;
    char op;
};

int main()
{
    int sockfd;
    struct sockaddr_in server;
    struct Message msg;
    char result[100];
    socklen_t len = sizeof(server);

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Server address
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (1)
    {
        printf("\n----- UDP CALCULATOR -----\n");
        printf("Enter first number: ");
        scanf("%f", &msg.num1);

        printf("Enter second number: ");
        scanf("%f", &msg.num2);

        printf("Enter operation (+, -, *, /) or e to exit: ");
        scanf(" %c", &msg.op);

        // Send data to server
        sendto(sockfd, &msg, sizeof(msg), 0,
               (struct sockaddr *)&server, len);
        recvfrom(sockfd, result, sizeof(result), 0,
                 (struct sockaddr *)&server, &len);
        printf("Server: %s\n", result);
        if (msg.op == 'e')
            break;
    }
    close(sockfd);
    return 0;
}

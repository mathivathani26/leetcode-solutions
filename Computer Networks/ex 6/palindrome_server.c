#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    char p[1024];
    char reply[1024];
    int sockid, clientid;
    struct sockaddr_in server, client;
    socklen_t client_len;

    sockid = socket(AF_INET, SOCK_STREAM, 0);

    if (sockid < 0)
    {
        perror("Socket failed");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(5100);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sockid, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Bind failed");
        close(sockid);
        return 1;
    }

    printf("Server IP   : 127.0.0.1\n");
    printf("Server Port : 5100\n");
    printf("Bind successful\n");

    if (listen(sockid, 5) < 0)
    {
        perror("Listen failed");
        close(sockid);
        return 1;
    }

    printf("Waiting for client...\n");

    client_len = sizeof(client);

    clientid = accept(sockid, (struct sockaddr *)&client, &client_len);

    if (clientid < 0)
    {
        perror("Accept failed");
        close(sockid);
        return 1;
    }

    printf("Client connected\n");

    while (1)
    {
        memset(p, 0, sizeof(p));

        int n = recv(clientid, p, sizeof(p) - 1, 0);

        if (n <= 0)
        {
            printf("Client disconnected\n");
            break;
        }

        p[n] = '\0';

        printf("Client: %s\n", p);

        if (strcmp(p, "busy") == 0)
        {
            printf("Closing connection...\n");
            break;
        }

        int i = 0;
        int j = strlen(p) - 1;
        int palindrome = 1;

        while (i < j)
        {
            if (p[i] != p[j])
            {
                palindrome = 0;
                break;
            }

            i++;
            j--;
        }

        if (palindrome)
            strcpy(reply, "Palindrome");
        else
            strcpy(reply, "Not Palindrome");

        send(clientid, reply, strlen(reply), 0);
    }

    close(clientid);
    close(sockid);

    return 0;
}

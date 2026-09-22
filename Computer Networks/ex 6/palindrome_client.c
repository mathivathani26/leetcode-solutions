#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    char p[1024];
    char reply[1024];

    int sockid;
    struct sockaddr_in server;

    sockid = socket(AF_INET, SOCK_STREAM, 0);

    if (sockid < 0)
    {
        perror("Socket failed");
        return 1;
    }

    printf("Socket successful\n");

    server.sin_family = AF_INET;
    server.sin_port = htons(5100);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Server IP   : 127.0.0.1\n");
    printf("Server Port : 5100\n");

    if (connect(sockid, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connection failed");
        close(sockid);
        return 1;
    }

    printf("Connected to server successfully!\n");

    while (1)
    {
        printf("Enter string: ");

        fgets(p, sizeof(p), stdin);
        p[strcspn(p, "\n")] = '\0';

        if (strlen(p) == 0)
            continue;

        send(sockid, p, strlen(p), 0);

        if (strcmp(p, "busy") == 0)
        {
            printf("Exiting client...\n");
            break;
        }

        memset(reply, 0, sizeof(reply));

        int n = recv(sockid, reply, sizeof(reply) - 1, 0);

        if (n > 0)
        {
            reply[n] = '\0';
            printf("Server: %s\n", reply);
        }
    }

    close(sockid);

    return 0;
}

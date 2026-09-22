#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>

/* Send complete data */
void send_all(int sock, void *data, int size)
{
    int sent = 0;
    int n;

    while (sent < size)
    {
        n = send(sock, (char *)data + sent, size - sent, 0);

        if (n <= 0)
            return;

        sent += n;
    }
}

/* Receive complete data */
int recv_all(int sock, void *data, int size)
{
    int received = 0;
    int n;

    while (received < size)
    {
        n = recv(sock, (char *)data + received,
                 size - received, 0);

        if (n <= 0)
            return 0;

        received += n;
    }

    return 1;
}

/* Send string with length */
void send_msg(int sock, char *msg)
{
    uint32_t len;

    len = strlen(msg);

    uint32_t netlen = htonl(len);

    send_all(sock, &netlen, sizeof(netlen));

    send_all(sock, msg, len);
}

/* Receive string with length */
int recv_msg(int sock, char *msg)
{
    uint32_t len;

    if (!recv_all(sock, &len, sizeof(len)))
        return 0;

    len = ntohl(len);

    if (len >= 1024)
        return 0;

    if (!recv_all(sock, msg, len))
        return 0;

    msg[len] = '\0';

    return 1;
}

int main()
{
    int sockid;

    struct sockaddr_in server;

    char buffer[1024];

    sockid = socket(AF_INET, SOCK_STREAM, 0);

    if (sockid < 0)
    {
        perror("Socket failed");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(5100);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockid,
                (struct sockaddr *)&server,
                sizeof(server)) < 0)
    {
        perror("Connection failed");
        close(sockid);
        return 1;
    }

    printf("Connected to server successfully!\n");

    while (1)
    {
        printf("\n===== STUDENT RESULT SYSTEM =====\n");

        printf("1. Display All Students\n");
        printf("2. Search Student\n");
        printf("3. Add Student\n");
        printf("4. Update Student\n");
        printf("5. Delete Student\n");
        printf("6. Class Topper\n");
        printf("7. Exit\n");

        printf("Enter choice: ");

        int choice;

        scanf("%d", &choice);

        sprintf(buffer, "%d", choice);

        send_msg(sockid, buffer);

        /* Display */

        if (choice == 1)
        {
            recv_msg(sockid, buffer);

            printf("\n%s\n", buffer);
        }

        /* Search */

        else if (choice == 2)
        {
            int regno;

            printf("Enter Register Number: ");

            scanf("%d", &regno);

            sprintf(buffer, "%d", regno);

            send_msg(sockid, buffer);

            recv_msg(sockid, buffer);

            printf("\n%s\n", buffer);
        }

        /* Add */

        else if (choice == 3)
        {
            getchar();

            printf("\nEnter:\n");
            printf("RegisterNo Name Department Semester Mark Grade CGPA\n");

            fgets(buffer, sizeof(buffer), stdin);

            buffer[strcspn(buffer, "\n")] = '\0';

            send_msg(sockid, buffer);

            recv_msg(sockid, buffer);

            printf("Server: %s\n", buffer);
        }

        /* Update */

        else if (choice == 4)
        {
            int regno;

            printf("Enter Register Number to update: ");

            scanf("%d", &regno);

            sprintf(buffer, "%d", regno);

            send_msg(sockid, buffer);

            recv_msg(sockid, buffer);

            printf("%s\n", buffer);

            getchar();

            printf("Enter new details:\n");
            printf("RegisterNo Name Department Semester Mark Grade CGPA\n");

            fgets(buffer, sizeof(buffer), stdin);

            buffer[strcspn(buffer, "\n")] = '\0';

            send_msg(sockid, buffer);

            recv_msg(sockid, buffer);

            printf("Server: %s\n", buffer);
        }

        /* Delete */

        else if (choice == 5)
        {
            int regno;

            printf("Enter Register Number to delete: ");

            scanf("%d", &regno);

            sprintf(buffer, "%d", regno);

            send_msg(sockid, buffer);

            recv_msg(sockid, buffer);

            printf("Server: %s\n", buffer);
        }

        /* Topper */

        else if (choice == 6)
        {
            recv_msg(sockid, buffer);

            printf("\n%s\n", buffer);
        }

        /* Exit */

        else if (choice == 7)
        {
            recv_msg(sockid, buffer);

            printf("Server: %s\n", buffer);

            break;
        }

        else
        {
            recv_msg(sockid, buffer);

            printf("Server: %s\n", buffer);
        }
    }

    close(sockid);

    return 0;
}

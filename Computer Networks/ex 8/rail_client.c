#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9999
#define MAX 1024

int main() {
    int sock;
    struct sockaddr_in server;
    char req[MAX], res[MAX];
    int choice, id, seats;
    char name[30];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server,
                sizeof(server)) < 0) {
        perror("Connection failed");
        return 1;
    }

    while (1) {
        printf("\n--- RAILWAY RESERVATION ---\n");
        printf("1. Search/View trains\n");
        printf("2. Check seat availability\n");
        printf("3. Book ticket\n");
        printf("4. Cancel booking\n");
        printf("5. Booking status\n");
        printf("0. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) break;

        if (choice == 1 || choice == 0) {
            sprintf(req, "%d", choice);
        }
        else if (choice == 2) {
            printf("Enter train ID: ");
            scanf("%d", &id);
            sprintf(req, "2 %d", id);
        }
        else if (choice == 3) {
            printf("Enter train ID, passenger name (one word), seats: ");
            scanf("%d %29s %d", &id, name, &seats);
            sprintf(req, "3 %d %s %d", id, name, seats);
        }
        else if (choice == 4 || choice == 5) {
            printf("Enter booking ID: ");
            scanf("%d", &id);
            sprintf(req, "%d %d", choice, id);
        }
        else {
            printf("Invalid choice.\n");
            continue;
        }

        send(sock, req, strlen(req), 0);

        memset(res, 0, sizeof(res));
        int n = recv(sock, res, sizeof(res) - 1, 0);
        if (n <= 0) break;
        res[n] = '\0';

        printf("\nServer: %s\n", res);

        if (choice == 0) break;
    }

    close(sock);
    return 0;
}

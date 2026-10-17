#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#define PORT 9999
#define MAX 1024

struct Train {
    int id;
    char name[30];
    char source[30];
    char destination[30];
    int seats;
    int fare;
};

struct Booking {
    int id;
    int trainId;
    char name[30];
    int seats;
    int active;
};

struct Train trains[] = {
    {1, "Pandian Express", "Madurai", "Chennai", 10, 500},
    {2, "Vaigai Express", "Madurai", "Chennai", 8, 400},
    {3, "Chennai Express", "Chennai", "Bangalore", 12, 350}
};

struct Booking bookings[100];
int bookingCount = 0;
int nextBookingId = 1001;
int trainCount = 3;

void logActivity(char *message) {
    FILE *fp = fopen("rail_log.txt", "a");
    if (fp != NULL) {
        time_t now = time(NULL);
        fprintf(fp, "%s : %s", ctime(&now), message);
        fclose(fp);
    }
}

void saveData() {
    FILE *fp = fopen("rail_data.txt", "w");
    if (fp == NULL) return;

    for (int i = 0; i < trainCount; i++)
        fprintf(fp, "%d %s %s %s %d %d\n",
            trains[i].id, trains[i].name,
            trains[i].source, trains[i].destination,
            trains[i].seats, trains[i].fare);

    fclose(fp);

    fp = fopen("rail_bookings.txt", "w");
    if (fp == NULL) return;

    for (int i = 0; i < bookingCount; i++)
        fprintf(fp, "%d %d %s %d %d\n",
            bookings[i].id, bookings[i].trainId,
            bookings[i].name, bookings[i].seats,
            bookings[i].active);

    fclose(fp);
}

void handleRequest(char *req, char *res) {
    int choice, id, seats, i, j;
    char name[30];

    if (sscanf(req, "%d", &choice) != 1) {
        strcpy(res, "Invalid request.");
        return;
    }

    if (choice == 1) {
        strcpy(res, "TRAIN LIST\n");

        for (i = 0; i < trainCount; i++) {
            char line[150];

            sprintf(line, "%d %s %s to %s Seats:%d Fare:%d\n",
                trains[i].id, trains[i].name,
                trains[i].source, trains[i].destination,
                trains[i].seats, trains[i].fare);

            strcat(res, line);
        }
    }
    else if (choice == 2) {
        if (sscanf(req, "%d %d", &choice, &id) != 2) {
            strcpy(res, "Invalid train ID.");
            return;
        }

        for (i = 0; i < trainCount; i++)
            if (trains[i].id == id) break;

        if (i == trainCount)
            strcpy(res, "Train not found.");
        else
            sprintf(res, "%s: Available seats = %d, Fare = %d",
                trains[i].name, trains[i].seats, trains[i].fare);
    }
    else if (choice == 3) {
        if (sscanf(req, "%d %d %29s %d",
                   &choice, &id, name, &seats) != 4 ||
            seats <= 0) {
            strcpy(res, "Invalid booking request.");
            return;
        }

        for (i = 0; i < trainCount; i++)
            if (trains[i].id == id) break;

        if (i == trainCount)
            strcpy(res, "Train not found.");
        else if (seats > trains[i].seats)
            strcpy(res, "Not enough seats available.");
        else if (bookingCount >= 100)
            strcpy(res, "Booking database is full.");
        else {
            bookings[bookingCount].id = nextBookingId++;
            bookings[bookingCount].trainId = id;
            strcpy(bookings[bookingCount].name, name);
            bookings[bookingCount].seats = seats;
            bookings[bookingCount].active = 1;

            trains[i].seats -= seats;

            sprintf(res, "Booking successful! Booking ID: %d Total Fare: %d",
                bookings[bookingCount].id, seats * trains[i].fare);

            bookingCount++;
            saveData();
        }
    }
    else if (choice == 4) {
        if (sscanf(req, "%d %d", &choice, &id) != 2) {
            strcpy(res, "Invalid booking ID.");
            return;
        }

        for (i = 0; i < bookingCount; i++)
            if (bookings[i].id == id && bookings[i].active)
                break;

        if (i == bookingCount)
            strcpy(res, "Active booking not found.");
        else {
            for (j = 0; j < trainCount; j++)
                if (trains[j].id == bookings[i].trainId) break;

            if (j < trainCount)
                trains[j].seats += bookings[i].seats;

            bookings[i].active = 0;
            strcpy(res, "Booking cancelled successfully.");
            saveData();
        }
    }
    else if (choice == 5) {
        if (sscanf(req, "%d %d", &choice, &id) != 2) {
            strcpy(res, "Invalid booking ID.");
            return;
        }

        for (i = 0; i < bookingCount; i++)
            if (bookings[i].id == id) break;

        if (i == bookingCount)
            strcpy(res, "Booking not found.");
        else
            sprintf(res, "Booking ID:%d Train ID:%d Passenger:%s Seats:%d Status:%s",
                bookings[i].id, bookings[i].trainId,
                bookings[i].name, bookings[i].seats,
                bookings[i].active ? "Confirmed" : "Cancelled");
    }
    else {
        strcpy(res, "Invalid menu choice.");
    }

    logActivity(req);
}

int main() {
    int server, client;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    char req[MAX], res[MAX];

    server = socket(AF_INET, SOCK_STREAM, 0);

    if (server < 0) {
        perror("Socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0 ||
        listen(server, 5) < 0) {
        perror("Server setup");
        close(server);
        return 1;
    }

    printf("Railway server running on port %d...\n", PORT);

    while (1) {
        client = accept(server, (struct sockaddr *)&addr, &len);
        if (client < 0) continue;

        printf("Client connected.\n");

        while (1) {
            memset(req, 0, sizeof(req));
            int n = recv(client, req, sizeof(req) - 1, 0);

            if (n <= 0) break;

            req[n] = '\0';
            memset(res, 0, sizeof(res));

            if (strcmp(req, "0") == 0) {
                strcpy(res, "Goodbye");
                send(client, res, strlen(res), 0);
                break;
            }

            handleRequest(req, res);
            send(client, res, strlen(res), 0);
        }

        close(client);
        printf("Client disconnected. Waiting for next client.\n");
    }

    close(server);
    return 0;
}

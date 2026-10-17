#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PORT 5000
#define MAX 1024

struct Student {
    char username[30];
    char password[30];
};

struct Student students[] = {
    {"mathi", "1234"},
    {"student1", "pass1"},
    {"student2", "pass2"}
};

int student_count = 3;

char *questions[] = {
    "1. Which language is used for system programming?\n"
    "1.C  2.HTML  3.CSS  4.SQL\n",

    "2. Which protocol is connection-oriented?\n"
    "1.UDP  2.TCP  3.IP  4.ARP\n",

    "3. Which data structure uses FIFO?\n"
    "1.Stack  2.Tree  3.Queue  4.Graph\n"
};

int correct[] = {1, 2, 3};

void log_activity(char *username, char *activity)
{
    FILE *fp = fopen("exam_log.txt", "a");

    if (fp != NULL) {
        time_t now = time(NULL);

        fprintf(fp, "%s | %s | %s",
                username, activity, ctime(&now));

        fclose(fp);
    }
}

int receive_line(int sock, char *buffer, int size)
{
    int i = 0;
    char ch;

    while (i < size - 1) {
        int n = recv(sock, &ch, 1, 0);

        if (n <= 0)
            return -1;

        if (ch == '\n')
            break;

        if (ch != '\r')
            buffer[i++] = ch;
    }

    buffer[i] = '\0';
    return i;
}

void send_text(int sock, const char *text)
{
    send(sock, text, strlen(text), 0);
}

void handle_client(int client, struct sockaddr_in addr)
{
    char username[30], password[30], buffer[MAX];
    char activity[100];
    int authenticated = 0;
    int i, answer, score = 0;

    char ip[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));

    printf("Client connected: IP=%s Port=%d\n",
           ip, ntohs(addr.sin_port));

    send_text(client, "ONLINE EXAMINATION SYSTEM\n");
    send_text(client, "Username: ");

    if (receive_line(client, username, sizeof(username)) <= 0)
        goto disconnect;

    send_text(client, "Password: ");

    if (receive_line(client, password, sizeof(password)) <= 0)
        goto disconnect;

    for (i = 0; i < student_count; i++) {
        if (strcmp(username, students[i].username) == 0 &&
            strcmp(password, students[i].password) == 0) {
            authenticated = 1;
            break;
        }
    }

    if (!authenticated) {
        send_text(client, "Invalid username or password.\n");
        log_activity(username, "Failed login");
        goto disconnect;
    }

    send_text(client, "Login successful!\n");
    log_activity(username, "Login successful");

    for (i = 0; i < 3; i++) {
        send_text(client, questions[i]);
        send_text(client, "Enter option (1-4): ");

        if (receive_line(client, buffer, sizeof(buffer)) <= 0)
            goto disconnect;

        answer = atoi(buffer);

        if (answer < 1 || answer > 4) {
            send_text(client, "Invalid option. No mark awarded.\n");
        } else if (answer == correct[i]) {
            score++;
            send_text(client, "Correct answer!\n");
        } else {
            send_text(client, "Wrong answer!\n");
        }

        snprintf(activity, sizeof(activity),
                 "Answered question %d", i + 1);

        log_activity(username, activity);
    }

    snprintf(buffer, sizeof(buffer),
             "Examination completed. Final score: %d/3\n",
             score);

    send_text(client, buffer);
    log_activity(username, "Examination completed");

disconnect:
    snprintf(activity, sizeof(activity),
             "Client disconnected: %s:%d", ip, ntohs(addr.sin_port));

    log_activity(username[0] ? username : "Unknown", activity);

    close(client);
    printf("Client disconnected: IP=%s Port=%d\n",
           ip, ntohs(addr.sin_port));
}

int main()
{
    int server, client;
    struct sockaddr_in address;
    socklen_t len;
    pid_t pid;

    signal(SIGCHLD, SIG_IGN);

    server = socket(AF_INET, SOCK_STREAM, 0);

    if (server < 0) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server, (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        perror("bind");
        close(server);
        exit(1);
    }

    if (listen(server, 10) < 0) {
        perror("listen");
        close(server);
        exit(1);
    }

    printf("Examination server running on port %d...\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        len = sizeof(client_addr);

        client = accept(server, (struct sockaddr *)&client_addr, &len);

        if (client < 0) {
            perror("accept");
            continue;
        }

        pid = fork();

        if (pid == 0) {
            close(server);
            handle_client(client, client_addr);
            exit(0);
        } else if (pid > 0) {
            close(client);
        } else {
            perror("fork");
            close(client);
        }
    }

    close(server);
    return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <time.h>

struct Student
{
    int regno;
    char name[50];
    char dept[20];
    int sem;
    int mark;
    char grade[5];
    float cgpa;
};

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

/* Send string with its length */
void send_msg(int sock, char *msg)
{
    uint32_t len;

    len = strlen(msg);

    len = htonl(len);

    send_all(sock, &len, sizeof(len));

    len = ntohl(len);

    send_all(sock, msg, len);
}

/* Receive string with its length */
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

/* Log activity */
void log_activity(char *msg)
{
    FILE *fp;
    time_t t;

    fp = fopen("result.log", "a");

    if (fp != NULL)
    {
        time(&t);

        fprintf(fp, "%s : %s", msg, ctime(&t));

        fclose(fp);
    }
}

/* Display all students */
void display_students(int clientid)
{
    FILE *fp;
    struct Student s;
    char buffer[1024];
    char temp[200];

    fp = fopen("students.txt", "r");

    if (fp == NULL)
    {
        send_msg(clientid, "No student records found");
        return;
    }

    strcpy(buffer, "");

    while (fscanf(fp, "%d %s %s %d %d %s %f",
                  &s.regno,
                  s.name,
                  s.dept,
                  &s.sem,
                  &s.mark,
                  s.grade,
                  &s.cgpa) == 7)
    {
        sprintf(temp,
                "RegNo:%d Name:%s Dept:%s Sem:%d Mark:%d Grade:%s CGPA:%.2f\n",
                s.regno,
                s.name,
                s.dept,
                s.sem,
                s.mark,
                s.grade,
                s.cgpa);

        strcat(buffer, temp);
    }

    fclose(fp);

    if (strlen(buffer) == 0)
        strcpy(buffer, "No student records found");

    send_msg(clientid, buffer);
}

/* Search student */
void search_student(int clientid)
{
    char buffer[1024];
    int regno;
    int found = 0;

    recv_msg(clientid, buffer);

    regno = atoi(buffer);

    FILE *fp = fopen("students.txt", "r");

    if (fp != NULL)
    {
        struct Student s;

        while (fscanf(fp, "%d %s %s %d %d %s %f",
                      &s.regno,
                      s.name,
                      s.dept,
                      &s.sem,
                      &s.mark,
                      s.grade,
                      &s.cgpa) == 7)
        {
            if (s.regno == regno)
            {
                sprintf(buffer,
                        "RegNo:%d Name:%s Dept:%s Sem:%d Mark:%d Grade:%s CGPA:%.2f",
                        s.regno,
                        s.name,
                        s.dept,
                        s.sem,
                        s.mark,
                        s.grade,
                        s.cgpa);

                found = 1;

                break;
            }
        }

        fclose(fp);
    }

    if (!found)
        strcpy(buffer, "Student not found");

    send_msg(clientid, buffer);

    log_activity("Student search performed");
}

/* Add student */
void add_student(int clientid)
{
    char buffer[1024];
    struct Student s;

    recv_msg(clientid, buffer);

    sscanf(buffer,
           "%d %s %s %d %d %s %f",
           &s.regno,
           s.name,
           s.dept,
           &s.sem,
           &s.mark,
           s.grade,
           &s.cgpa);

    FILE *fp = fopen("students.txt", "a");

    if (fp == NULL)
    {
        send_msg(clientid, "Unable to open student file");
        return;
    }

    fprintf(fp,
            "%d %s %s %d %d %s %.2f\n",
            s.regno,
            s.name,
            s.dept,
            s.sem,
            s.mark,
            s.grade,
            s.cgpa);

    fclose(fp);

    send_msg(clientid, "Student added successfully");

    log_activity("Student added");
}

/* Update student */
void update_student(int clientid)
{
    char buffer[1024];
    int regno;
    int found = 0;

    recv_msg(clientid, buffer);

    regno = atoi(buffer);

    /* Ask for new data */
    send_msg(clientid, "Enter new student details:");

    recv_msg(clientid, buffer);

    struct Student new_student;

    sscanf(buffer,
           "%d %s %s %d %d %s %f",
           &new_student.regno,
           new_student.name,
           new_student.dept,
           &new_student.sem,
           &new_student.mark,
           new_student.grade,
           &new_student.cgpa);

    FILE *fp = fopen("students.txt", "r");
    FILE *temp = fopen("temp.txt", "w");

    if (fp == NULL || temp == NULL)
    {
        send_msg(clientid, "File error");
        return;
    }

    struct Student s;

    while (fscanf(fp, "%d %s %s %d %d %s %f",
                  &s.regno,
                  s.name,
                  s.dept,
                  &s.sem,
                  &s.mark,
                  s.grade,
                  &s.cgpa) == 7)
    {
        if (s.regno == regno)
        {
            new_student.regno = regno;

            fprintf(temp,
                    "%d %s %s %d %d %s %.2f\n",
                    new_student.regno,
                    new_student.name,
                    new_student.dept,
                    new_student.sem,
                    new_student.mark,
                    new_student.grade,
                    new_student.cgpa);

            found = 1;
        }
        else
        {
            fprintf(temp,
                    "%d %s %s %d %d %s %.2f\n",
                    s.regno,
                    s.name,
                    s.dept,
                    s.sem,
                    s.mark,
                    s.grade,
                    s.cgpa);
        }
    }

    fclose(fp);
    fclose(temp);

    remove("students.txt");
    rename("temp.txt", "students.txt");

    if (found)
        send_msg(clientid, "Student updated successfully");
    else
        send_msg(clientid, "Student not found");

    log_activity("Student update performed");
}

/* Delete student */
void delete_student(int clientid)
{
    char buffer[1024];
    int regno;
    int found = 0;

    recv_msg(clientid, buffer);

    regno = atoi(buffer);

    FILE *fp = fopen("students.txt", "r");
    FILE *temp = fopen("temp.txt", "w");

    if (fp == NULL || temp == NULL)
    {
        send_msg(clientid, "File error");
        return;
    }

    struct Student s;

    while (fscanf(fp, "%d %s %s %d %d %s %f",
                  &s.regno,
                  s.name,
                  s.dept,
                  &s.sem,
                  &s.mark,
                  s.grade,
                  &s.cgpa) == 7)
    {
        if (s.regno == regno)
        {
            found = 1;
            continue;
        }

        fprintf(temp,
                "%d %s %s %d %d %s %.2f\n",
                s.regno,
                s.name,
                s.dept,
                s.sem,
                s.mark,
                s.grade,
                s.cgpa);
    }

    fclose(fp);
    fclose(temp);

    remove("students.txt");
    rename("temp.txt", "students.txt");

    if (found)
        send_msg(clientid, "Student deleted successfully");
    else
        send_msg(clientid, "Student not found");

    log_activity("Student deletion performed");
}

/* Class topper */
void class_topper(int clientid)
{
    FILE *fp;

    struct Student s;
    struct Student topper;

    int found = 0;

    char buffer[1024];

    fp = fopen("students.txt", "r");

    if (fp == NULL)
    {
        send_msg(clientid, "No records found");
        return;
    }

    while (fscanf(fp, "%d %s %s %d %d %s %f",
                  &s.regno,
                  s.name,
                  s.dept,
                  &s.sem,
                  &s.mark,
                  s.grade,
                  &s.cgpa) == 7)
    {
        if (!found || s.cgpa > topper.cgpa)
        {
            topper = s;
            found = 1;
        }
    }

    fclose(fp);

    if (found)
    {
        sprintf(buffer,
                "Class Topper\n"
                "RegNo:%d\n"
                "Name:%s\n"
                "Dept:%s\n"
                "Sem:%d\n"
                "Mark:%d\n"
                "Grade:%s\n"
                "CGPA:%.2f",
                topper.regno,
                topper.name,
                topper.dept,
                topper.sem,
                topper.mark,
                topper.grade,
                topper.cgpa);
    }
    else
    {
        strcpy(buffer, "No records found");
    }

    send_msg(clientid, buffer);

    log_activity("Class topper requested");
}

int main()
{
    int sockid;
    int clientid;

    struct sockaddr_in server;
    struct sockaddr_in client;

    socklen_t client_len;

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

    if (bind(sockid,
             (struct sockaddr *)&server,
             sizeof(server)) < 0)
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

    clientid = accept(sockid,
                      (struct sockaddr *)&client,
                      &client_len);

    if (clientid < 0)
    {
        perror("Accept failed");
        close(sockid);
        return 1;
    }

    printf("Client connected\n");

    while (1)
    {
        if (!recv_msg(clientid, buffer))
            break;

        if (strcmp(buffer, "1") == 0)
        {
            display_students(clientid);
            log_activity("Displayed all students");
        }

        else if (strcmp(buffer, "2") == 0)
        {
            search_student(clientid);
        }

        else if (strcmp(buffer, "3") == 0)
        {
            add_student(clientid);
        }
        else if (strcmp(buffer, "4") == 0)
        {
            update_student(clientid);
        }
        else if (strcmp(buffer, "5") == 0)
        {
            delete_student(clientid);
        }
        else if (strcmp(buffer, "6") == 0)
        {
            class_topper(clientid);
        }
        else if (strcmp(buffer, "7") == 0)
        {
            send_msg(clientid, "Session terminated");
            log_activity("Client disconnected");
            break;
        }
        else
        {
            send_msg(clientid, "Invalid operation");
            log_activity("Invalid operation");
        }
    }
    close(clientid);
    close(sockid);
    return 0;
}

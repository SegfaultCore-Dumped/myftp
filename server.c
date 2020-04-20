/*
** EPITECH PROJECT, 2020
** server.c
** File description:
** server.c
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/stat.h>

#define MAX 1024

typedef struct command_s
{
    char *cmd;
    void (*func)(int, char*);
} command_t;

char *pathname(int sd, char *buffer)
{
    int i = 0;
    int j = 0;
    char *str = malloc(sizeof(char) * 80);

    sd = sd;
    for (i = 0; buffer[i] != ' '; i++);
    for (i = i + 1; buffer[i] != '\0'; i++)
        str[j++] = buffer[i];
    return (str);
}

void cwd(int sd, char *buffer)
{
    char *path = pathname(sd, buffer);

    if (chdir(path) == 0)
        write(sd, "250 Requested file action okay, completed\n", 42);
    else
        write(sd, "550 Requested action not taken\n", 31);
    free(path);
}

void dele(int sd, char *buffer)
{
    char *path = pathname(sd, buffer);

    if (remove(path) == 0)
        write(sd, "250 Requested file action okay, completed\n", 42);
    else
        write(sd, "550 Requested action not taken\n", 31);
    free(path);
}

void cdup(int sd, char *buffer)
{
    buffer = buffer;
    if (chdir("..") == 0)
        write(sd, "200 Command okay\n", 17);
    else
        write(sd, "500 Syntax error, command unrecognized\n", 39);
}

void help(int sd, char *buffer)
{
    buffer = buffer;
    write(sd, "214 Help message\n", 17);
}

void noop(int sd, char *buffer)
{
    buffer = buffer;
    write(sd, "200 Command okay\n", 17);
}

void pwd(int sd, char *buffer)
{
    char str[1024];
    char *add = "257 \"";
    char *result;

    sd = sd;
    buffer = buffer;
    getcwd(str, sizeof(str));
    result = malloc(strlen(str) + strlen("\n")
                    + strlen("257 \"") + 1);
    strcpy(result, add);
    strcat(result, str);
    strcat(result, "\" created\n");
    write(sd, result, strlen(result));
    free(result);
}

command_t command[] = {{"PWD", pwd},
                       {"NOOP", noop},
                       {"HELP", help},
                       {"CDUP", cdup},
                       {"DELE", dele},
                       {"CWD", cwd}};

int check_command(int sd, char *buffer, char *user)
{
    if (strcmp("PWD", user) != 0
        && strcmp("NOOP", user) != 0
        && strcmp("HELP", user) != 0
        && strcmp("QUIT", user) != 0
        && strcmp("CDUP", user) != 0
        && strcmp("CWD", user) != 0
        && strcmp("DELE", user) != 0)
        return (1);
    for (command_t *cmd = command; cmd != command
             + sizeof(command)
             / sizeof(command[0]); cmd++) {
        if(!strcmp(cmd->cmd, user)) {
            (*cmd->func)(sd, buffer);
            break;
        }
    }
    return (0);
}

char *password(int sd, char *buffer, char *rts)
{
    char *string = malloc(sizeof(char) * 80);

    strcpy(string, buffer);
    printf("pass: %s\n", rts);
    if (strncmp(rts, "USER Anonymous", 14) == 0)
        write(sd, "230 User logged in, proceed\n", 28);
    else
        write(sd, "530 Not logged in\n", 18);
    return (string);
}

char *username(int sd, char *buffer)
{
    char *rts = malloc(sizeof(char) * 80);

    strcpy(rts, buffer);
    write(sd, "331 User name okay, need password\n", 34);
    return (rts);
}

char *parseuser(char *buff)
{
    char *token = strtok(buff, "A");

    while (token != NULL)
        token = strtok(NULL, "A");
    return (buff);
}

char *parsecommand(char *buff)
{
    char *token = strtok(buff, " ");

    while (token != NULL)
        token = strtok(NULL, " ");
    return (buff);
}

void strip_extra_spaces(char* str)
{
    int i;
    int x;

    for(i=x=0; str[i]; ++i)
        if(!isspace(str[i])
           || (i > 0 && !isspace(str[i-1])))
            str[x++] = str[i];
    str[x] = '\0';
}

char *parsecmd(char *buff)
{
    char *token = strtok(buff, "\t\r\n");

    while (token != NULL)
        token = strtok(NULL, "\t\r\n");
    strip_extra_spaces(buff);
    return (buff);
}

int arguments(int ac, char **av)
{
    if (ac > 3 || ac < 3)
        return (84);
    ac = ac;
    if (chdir(av[2]) == -1) {
        perror(av[2]);
        return (84);
    }
    return (0);
}

void quit(int sd, struct sockaddr_in address, int addrlen)
{
    getpeername(sd, (struct sockaddr*)&address,
                (socklen_t*)&addrlen);
    printf("Host disconnected , ip %s , port %d\n",
           inet_ntoa(address.sin_addr),
           ntohs(address.sin_port));
    write(sd, "221 Service closing control connection\n", 39);
    close(sd);
}

int new_connection(int sockfd, struct sockaddr_in address, int addrlen)
{
    int new_sockfd;

    if ((new_sockfd = accept(sockfd,
                             (struct sockaddr *)&address,
                             (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        return (84);
    }
    printf("New connection , socket fd is %d , ip is : %s , port : %d\n",
           new_sockfd, inet_ntoa(address.sin_addr),
           ntohs(address.sin_port));
    write(new_sockfd, "220 Service ready for new user\n", 31);
    return (new_sockfd);
}

int server_socket(int ac, char **av)
{
    int sockfd;
    struct sockaddr_in address;

    if (arguments(ac, av) == 84)
        return (84);
    printf("Correct number of arguments\n");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        return (84);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(av[1]));
    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind\n");
        return (84);
    }
    printf("Listening port %d\n", atoi(av[1]));
    if (listen(sockfd, 3) < 0)
        return (84);
    return (sockfd);
}

int main(int ac, char **av)
{
    int sockfd, addrlen, new_sockfd, clients[FD_SETSIZE], i, max_sd;
    struct sockaddr_in addr;
    char *rts = malloc(sizeof(char) * 80);
    char *string = malloc(sizeof(char) * 80);
    char *user = malloc(sizeof(char) * 80);
    char *command = malloc(sizeof(char) * 80);
    char buff[MAX];
    fd_set readfds;

    sockfd = server_socket(ac, av);
    for (i = 0; i < FD_SETSIZE; i++)
        clients[i] = 0;
    addrlen = sizeof(addr);
    for (;;) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        max_sd = sockfd;
        for (i = 0 ; i < FD_SETSIZE; i++) {
            if(clients[i] > 0)
                FD_SET(clients[i], &readfds);
            if(clients[i] > max_sd)
                max_sd = clients[i];
        }
        if (select(max_sd + 1, &readfds, NULL, NULL, NULL) < 0)
            return (84);
        if (FD_ISSET(sockfd, &readfds)) {
            if ((new_sockfd = new_connection(sockfd, addr, addrlen)) == 84)
                return (84);
            for (i = 0; i < FD_SETSIZE; i++) {
                if (clients[i] == 0) {
                    clients[i] = new_sockfd;
                    break;
                }
            }
        }
        for (i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(clients[i], &readfds)) {
                if ((read(clients[i], buff, sizeof(buff))) <= 0) {
                    quit(clients[i], addr, addrlen);
                    clients[i] = 0;
                } else {
                    parsecmd(buff);
                    printf("%s\n", buff);
                    strcpy(user, buff);
                    parseuser(user);
                    strcpy(command, buff);
                    parsecommand(command);
                    if (strncmp(buff, "USER Anonymous", 14) == 0
                        || strncmp(user, "USER ", 4) == 0)
                        rts = username(clients[i], buff);
                    else if (strncmp(buff, "PASS ", 5) == 0) {
                        string = password(clients[i], buff, rts);
                    }
                    else if (strncmp(buff, "QUIT", 4) == 0) {
                        quit(clients[i], addr, addrlen);
                        clients[i] = 0;
                        break;
                    }
                    else if (strncmp(rts, "USER Anonymous", 14) == 0) {
                        if (strncmp(string, "PASS ", 5) == 0) {
                            if (strncmp(buff, "QUIT", 4) == 0) {
                                quit(clients[i], addr, addrlen);
                                clients[i] = 0;
                                break;
                            }
                            else if (check_command(clients[i], buff, command) == 1)
                                write(clients[i], "500 Syntax error, command unrecognized\n", 39);
                        }
                    } else
                        write(clients[i], "530 Not logged in\n", 18);
                    bzero(buff, sizeof(buff));
                    FD_CLR(clients[i], &readfds);
                }
            }
        }
    }

    return (0);
}

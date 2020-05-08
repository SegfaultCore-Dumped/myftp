/*
** EPITECH PROJECT, 2020
** server.c
** File description:
** server.c
*/

#include "server.h"

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

    for(i = x = 0; str[i]; ++i)
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

int quit(server_t *s, int i)
{
    getpeername(s->clients[i], (struct sockaddr*)&s->addr,
                (socklen_t*)&s->addrlen);
    printf("Host disconnected , ip %s , port %d\n",
           inet_ntoa(s->addr.sin_addr),
           ntohs(s->addr.sin_port));
    write(s->clients[i], "221 Service closing control connection\n", 39);
    close(s->clients[i]);
    s->clients[i] = 0;
    return (1);
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

int server_socket(server_t *s, int i, int ac, char **av)
{
    if (arguments(ac, av) == 84)
        return (84);
    printf("Correct number of arguments\n");
    if ((s->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return (84);
    s->addr.sin_family = AF_INET;
    s->addr.sin_addr.s_addr = INADDR_ANY;
    s->addr.sin_port = htons(atoi(av[1]));
    if (bind(s->sockfd, (struct sockaddr *)&s->addr, sizeof(s->addr)) < 0) {
        perror("bind");
        return (84);
    }
    printf("Listening port %d\n", atoi(av[1]));
    if (listen(s->sockfd, 3) < 0)
        return (84);
    for (i = 0; i < FD_SETSIZE; i++)
        s->clients[i] = 0;
    s->addrlen = sizeof(s->addr);
    return (0);
}

int select_new_connection(server_t *s, int i)
{
    s->max_sd = s->sockfd;
    for (i = 0 ; i < FD_SETSIZE; i++) {
        if(s->clients[i] > 0)
            FD_SET(s->clients[i], &s->readfds);
        if(s->clients[i] > s->max_sd)
            s->max_sd = s->clients[i];
    }
    if (select(s->max_sd + 1, &s->readfds, NULL, NULL, NULL) < 0)
        return (84);
    if (FD_ISSET(s->sockfd, &s->readfds)) {
        if ((s->new_sockfd = new_connection(s->sockfd, s->addr, s->addrlen)) == 84)
            return (84);
        for (i = 0; i < FD_SETSIZE; i++) {
            if (s->clients[i] == 0) {
                s->clients[i] = s->new_sockfd;
                break;
            }
        }
    }
    return (0);
}

int login_and_commands(server_t *s, int i)
{
    if (strncmp(s->buff, "USER Anonymous", 14) == 0
        || strncmp(s->user, "USER ", 4) == 0)
        s->rts = username(s->clients[i], s->buff);
    else if (strncmp(s->buff, "PASS ", 5) == 0)
        s->string = password(s->clients[i], s->buff, s->rts);
    else if (strncmp(s->buff, "QUIT", 4) == 0)
        return (quit(s, i));
    else if (strncmp(s->rts, "USER Anonymous", 14) == 0) {
        if (strncmp(s->string, "PASS ", 5) == 0) {
            if (strncmp(s->buff, "QUIT", 4) == 0)
                return (quit(s, i));
            else if (check_command(s->clients[i], s->buff, s->command) == 1)
                write(s->clients[i],
                      "500 Syntax error, command unrecognized\n", 39);
        }
    } else
        write(s->clients[i], "530 Not logged in\n", 18);
    return (0);
}

int server_system(server_t *s, int i)
{
    if ((read(s->clients[i], s->buff, sizeof(s->buff))) <= 0) {
        return (quit(s, i));
    } else {
        parsecmd(s->buff);
        printf("%s\n", s->buff);
        strcpy(s->user, s->buff);
        parseuser(s->user);
        strcpy(s->command, s->buff);
        parsecommand(s->command);
        if ((login_and_commands(s, i)) == 1)
            return (1);
        bzero(s->buff, sizeof(s->buff));
        FD_CLR(s->clients[i], &s->readfds);
    }
    return (0);
}

int main(int ac, char **av)
{
    server_t *s = malloc(sizeof(server_t));
    int i = 0;

    s->rts = malloc(sizeof(char) * 80);
    s->string = malloc(sizeof(char) * 80);
    s->user = malloc(sizeof(char) * 80);
    s->command = malloc(sizeof(char) * 80);
    if ((server_socket(s, i, ac, av)) == 84)
        return (84);
    for (;;) {
        FD_ZERO(&s->readfds);
        FD_SET(s->sockfd, &s->readfds);
        if ((select_new_connection(s, i)) == 84)
            return (84);
        for (i = 0; i < FD_SETSIZE; i++)
            if (FD_ISSET(s->clients[i], &s->readfds))
                if ((server_system(s, i)) == 1)
                    break;
    }
    return (0);
}

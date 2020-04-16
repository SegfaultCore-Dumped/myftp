/*
** EPITECH PROJECT, 2020
** client.c
** File description:
** client.c
*/

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX 80

void strip_extra_spaces(char* str)
{
    int i, x;

    for(i=x=0; str[i]; ++i)
        if(!isspace(str[i]) || (i > 0 && !isspace(str[i-1])))
            str[x++] = str[i];
    str[x] = '\0';
}

char *parsecmd(char *buff)
{
    char *token = strtok(buff, "\t\r\n");
    while (token != NULL) {
        token = strtok(NULL, "\t\r\n");
    }
    strip_extra_spaces(buff);
    return (buff);
}

int main(int ac, char const **av)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in client;
    char buff[MAX];
    char *line = NULL;
    size_t len = 0;
    ssize_t read_line;

    ac = ac;
    if (sockfd == -1)
        return (84);
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(av[1]);
    client.sin_port = htons(atoi(av[2]));
    if (connect(sockfd, (const struct sockaddr *)&client, sizeof(client)) == -1)
        return (84);
    while (42) {
        read(sockfd, buff, MAX);
        printf("%s", buff);
        while ((read_line = getline(&line, &len, stdin)) != -1) {
            parsecmd(line);
            write(sockfd, line, strlen(line) + 2);
            read(sockfd, buff, sizeof(buff));
            if (strncmp(line, "QUIT", 4) == 0) {
                close(sockfd);
                return (0);
            }
            printf("%s", buff);
        }
    }
    return (0);
}

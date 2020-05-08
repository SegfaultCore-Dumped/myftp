/*
** EPITECH PROJECT, 2020
** server.h
** File description:
** server.h
*/

#ifndef _SERVER_H_
#define _SERVER_H_

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

typedef struct server_s
{
    int sockfd;
    int addrlen;
    int new_sockfd;
    int clients[FD_SETSIZE];
    int max_sd;
    struct sockaddr_in addr;
    char buff[MAX];
    fd_set readfds;
    char *rts;
    char *string;
    char *user;
    char *command;
} server_t;

#endif

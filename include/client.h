/*
** EPITECH PROJECT, 2020
** client.h
** File description:
** client.h
*/

#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX 80

void strip_extra_spaces(char *);
char *parsecmd(char *);

#endif

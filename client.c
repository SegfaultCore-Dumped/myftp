// Client side C/C++ program to demonstrate Socket programming 
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

char *parseCmd(char *buff)
{ 
    char *token = strtok(buff, "\t\r\n");   
    // Keep printing tokens while one of the 
    // delimiters present in str[]. 
    while (token != NULL) { 
        //printf("%s", token); 
        token = strtok(NULL, "\t\r\n"); 
    }
    strip_extra_spaces(buff);
    return (buff);
}

char *parseUser(char *buff)
{ 
    char *token = strtok(buff, "A"); 
    while (token != NULL) { 
        //printf("%s", token);
        token = strtok(NULL, "A"); 
    }
    //strip_extra_spaces(buff);
    return (buff);
}

int main(int ac, char const **av) 
{ 
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //int nb_read;
    struct sockaddr_in client;
    char buff[MAX];
    // int i;
    char *line = NULL;  /* forces getline to allocate with malloc */
    size_t len = 0;     /* ignored when line = NULL */
    ssize_t read_line;
    // bool user = false;
    //bool pass = false;
    char *str = malloc(sizeof(char) * 80);
    char *rts = malloc(sizeof(char) * 80);
    char *user = malloc(sizeof(char) * 80);

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
            //write(sockfd, buff, strlen(line));
            //if (strncmp(line, "USER Anonymous\n", 15) == 0) {
            parseCmd(line);
            strcpy(user, line);
            parseUser(user);
            write(sockfd, line, strlen(line) + 2);
            //printf("after parse: %s\n", line);
            if (strncmp(buff, "USER Anonymous", 14) == 0 || strncmp(user, "USER", 4) == 0) {
                strcpy(rts, line);
                read(sockfd, buff, MAX);
                printf("%s", buff);
            }
            else if (strncmp(line, "PASS ", 5) == 0) {
                strcpy(str, line);
                if (strncmp(user, rts, 14) != 0) {
                    read(sockfd, buff, MAX);
                    printf("%s", buff);
                } else {
                    read(sockfd, buff, MAX);
                    printf("%s", buff);
                }
            }
            else if (strncmp(rts, "USER Anonymous", 14) == 0) {
                if (strncmp(str, "PASS ", 5) == 0) {
                    if (strncmp(line, "QUIT", 4) == 0) {
                        close(sockfd);
                        return (0);
                    }
                    else if (strncmp(line, "PWD", 3) == 0) {
                        read(sockfd, buff, MAX);
                        printf("%s", buff);
                    } else {
                        read(sockfd, buff, MAX);
                        printf("%s", buff);
                    }
                }
            } else {
                read(sockfd, buff, MAX);
                printf("%s", buff);
            }





            
            /* if (strncmp(user, "USER", 4) == 0) {
                strcpy(user, buffer);
                read(sockfd, buff, MAX);
                printf("%s", buff);
            }
            if (strncmp(line, "USER Anonymous", 14) == 0) {
                strcpy(str, line);
                read(sockfd, buff, MAX);
                printf("%s", buff);
                user = true;
            }
            else if (user == true) {
                if (strncmp(line, "PASS ", 5) == 0) {
                    strcpy(rts, line);
                    read(sockfd, buff, MAX);
                    printf("%s", buff);
                    pass = true;
                }
            }
            else if (pass == true) {
                if (strncmp(line, "QUIT", 4) == 0) {
                    close(sockfd);
                    return (0);
                }
                if (strncmp(line, "PWD", 3) == 0) {
                    read(sockfd, buff, MAX);
                    printf("%s", buff);
                }
            } else {
                read(sockfd, buff, MAX);
                printf("%s", buff);
                }*/
            /*if (pass == true) {
                if (strncmp(str, "USER Anonymous", 14) == 0) {
                    if (strncmp(rts, "PASS ", 5) == 0) {
                        if (strncmp(line, "QUIT", 4) == 0) {
                            close(sockfd);
                            return (0);
                        }
                        if (strncmp(line, "PWD", 3) == 0) {
                            read(sockfd, buff, MAX);
                            printf("%s", buff);
                        }
                    }
                }
                }*/
        }
    }
    return 0; 
} 

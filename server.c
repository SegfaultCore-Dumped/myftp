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
    while (token != NULL) { 
        //printf("%s", token); 
        token = strtok(NULL, "\t\r\n"); 
    }
    strip_extra_spaces(buff);
    return (buff);
}


int main(int ac, char **av)
{
    //fd_set master;
    fd_set readfds;
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    int max_sd;
    int client_socket[30];
    int max_clients = 30;
    int sd;
    int newfd;
    unsigned int addrlen;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    char buff[MAX];
    const char *str;
    int i = 0;
    bool user = false;
    char *rts = malloc(sizeof(char) * 80);
    char *string = malloc(sizeof(char) * 80);
    
    for (i = 0; i < max_clients; i++) 
    {
        client_socket[i] = 0;
    }
    ac = ac;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(atoi(av[1]));

    if(bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("Server-bind() error lol!");
        return (84);
    }
    if(listen(sockfd, 3) == -1) {
        perror("Server-listen() error lol!");
        return (84);
    }
    addrlen = sizeof(serveraddr);
    for(;;) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        max_sd = sockfd;
        //read_fds = master;
        for ( i = 0 ; i < max_clients ; i++) 
        {
            //socket descriptor
            sd = client_socket[i];
             
            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);
             
            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }
        if(select(max_sd + 1, &readfds, NULL, NULL, NULL) == -1)
            return (84);
/*run through the existing connections looking for data to be read*/
        //for(i = 0; i <= fdmax; i++) {
        if(FD_ISSET(sockfd, &readfds)) { /* we got one... */
            //if(i == sockfd) {
                /* handle new connections */
                //addrlen = sizeof(clientaddr);
            if((newfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
                return (84);
            /*FD_SET(newfd, &master);
            if (newfd > fdmax)
            fdmax = newfd;*/
            write(newfd, "220 Service ready for new user\n", 31);
            printf("New connection from %s on socket %d\n", inet_ntoa(clientaddr.sin_addr), newfd);
            
            while (42) {
                read(newfd, buff, sizeof(buff));
                parseCmd(buff);
                printf("%s\n", buff);
                if (strncmp(buff, "USER Anonymous", 14) == 0) {
                    strcpy(rts, buff);
                    write(newfd, "331 User name okay, need password\n", 34);
                    user = true;
                }
                else if (user == true) {
                    if (strncmp(buff, "PASS ", 5) == 0) {
                        strcpy(string, buff);
                        write(newfd, "230 User logged in, proceed\n", 28);
                    }
                } else
                    write(newfd, "530 Permission denied\n", 22);
                if (strncmp(rts, "USER Anonymous", 14) == 0) {
                    if (strncmp(string, "PASS ", 5) == 0) {
                        if (strncmp(buff, "QUIT", 4) == 0) {
                            printf("Client is gone\n");
                            close(newfd);
                            break;
                        }
                        if (strncmp(buff, "PWD", 3) == 0) {
                            str = getenv("PWD");
                            char *result = malloc(strlen(str) + strlen("\n") + 2);
                            strcpy(result, str);
                            strcat(result, "\n");
                            write(newfd, result, strlen(result) + 2);
                        }
                    }
                }
                bzero(buff, MAX);
            }
            for (i = 0; i < max_clients; i++) {
                //if position is empty
                if( client_socket[i] == 0 ) {
                    client_socket[i] = newfd;
                    printf("Adding to list of sockets as %d\n" , i);
                    break;
                }
            }   
        }
    }
    return (0);
}

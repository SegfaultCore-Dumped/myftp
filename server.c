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

char *pwd(void)
{
    const char *str = getenv("PWD");
    char *add = "257 \"";
    char *result = malloc(strlen(str) + strlen("\n")
                          + strlen("257 \"") + 1);
    strcpy(result, add);
    strcat(result, str);
    strcat(result, "\" created\n");
    return (result);
}

void quit(int sd, struct sockaddr_in address, int addrlen)
{
    getpeername(sd, (struct sockaddr*)&address,
                (socklen_t*)&addrlen);
     printf("Host disconnected , ip %s , port %d\n",
            inet_ntoa(address.sin_addr),
            ntohs(address.sin_port));
    close(sd);
}

char *password(int sd, char *buffer, char *rts)
{
    char *string = malloc(sizeof(char) * 80);

    strcpy(string, buffer);
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
    while (token != NULL)
        token = strtok(NULL, "\t\r\n");
    strip_extra_spaces(buff);
    return (buff);
}

int main(int ac, char **av)
{
    int master_socket , addrlen , new_socket , client_socket[30],
        max_clients = 30 , activity, i , valread , sd;
    int max_sd;
    struct sockaddr_in address;
    char *rts = malloc(sizeof(char) * 80);
    char *string = malloc(sizeof(char) * 80);
    char *user = malloc(sizeof(char) * 80);
    char buffer[1025];
    fd_set readfds;

    if (ac > 3 || ac < 3)
        return (84);
    if (chdir(av[2]) == -1) {
        perror(av[2]);
        return (84);
    }
    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
        client_socket[i] = 0;
    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) {
        perror("socket failed");
        return (84);
    }
    //set master socket to allow multiple connections
    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(av[1]));
    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return (84);
    }
    printf("Listener on port %d\n", atoi(av[1]));
    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0) {
        perror("listen");
        return (84);
    }
    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while(42) {
        //clear the socket set
        FD_ZERO(&readfds);
        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++) {
            //socket descriptor
            sd = client_socket[i];
            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET(sd, &readfds);
            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }
        //wait for an activity on one of the sockets, timeout is NULL,
        //so wait indefinitely
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno!=EINTR))
            printf("select error");
        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds)) {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
            write(new_socket, "220 Service ready for new user\n", 31);
            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++) {
                //if position is empty
                if( client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n" , i);
                    break;
                }
            }
        }
        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if (FD_ISSET(sd , &readfds)) {
                if ((valread = read(sd , buffer, sizeof(buffer))) <= 0) {
                    quit(sd, address, addrlen);
                    client_socket[i] = 0;
                } else {
                    parsecmd(buffer);
                    printf("%s\n", buffer);
                    strcpy(user, buffer);
                    parseuser(user);
                    // printf("%s\n", user);
                    if (strncmp(buffer, "USER Anonymous", 14) == 0
                        || strncmp(user, "USER ", 4) == 0)
                        rts = username(sd, buffer);
                    else if (strncmp(buffer, "PASS ", 5) == 0)
                        string = password(sd, buffer, rts);
                    else if (strncmp(buffer, "QUIT", 4) == 0) {
                        quit(sd, address, addrlen);
                        client_socket[i] = 0;
                        break;
                    }
                    else if (strncmp(rts, "USER Anonymous", 14) == 0) {
                        if (strncmp(string, "PASS ", 5) == 0) {
                            if (strncmp(buffer, "QUIT", 4) == 0) {
                                quit(sd, address, addrlen);
                                client_socket[i] = 0;
                                break;
                            }
                            else if (strncmp(buffer, "PWD", 3) == 0) {
                                write(sd, pwd(), strlen(pwd()));
                            }
                            else if (strncmp(buffer, "NOOP", 4) == 0)
                                 write(sd, "200 Command okay\n", 17);
                            else if (strncmp(buffer, "HELP", 4) == 0) {
                                 write(sd, "214 Help message\n", 17);
                                 write(sd, "HELP NOOP PWD QUIT USER\n", 24);
                            }
                        }
                    } else
                        write(sd, "500 Syntax error, command unrecognized\n", 39);
                    bzero(buffer, sizeof(buffer));
                    FD_CLR(sd, &readfds);
                }
            }
        }
    }

    return 0;
}

##
## EPITECH PROJECT, 2020
## Makefile
## File description:
## Makefile
##

SRC_SERVEUR	=	src/server.c

SRC_CLIENT	=	src/client.c

OBJ_SERVEUR	=	$(SRC_SERVEUR:.c=.o)

OBJ_CLIENT	=	$(SRC_CLIENT:.c=.o)

NAME_SERVEUR	=	myftp

NAME_CLIENT	=	client

CC	=	gcc

CFLAGS	=	-W -Wall -Wextra

CFLAGS	+=	-I./include/

RM	=	rm -rf

all:	
	@$(CC) -o $(NAME_SERVEUR) $(CFLAGS) $(SRC_SERVEUR)
	@$(CC) -o $(NAME_CLIENT) $(CFLAGS) $(SRC_CLIENT)

clean:
	@$(RM) *~ *# *.o

fclean: clean
	@$(RM) $(NAME_SERVEUR)
	@$(RM) $(NAME_CLIENT)

re:	fclean all

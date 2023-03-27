NAME = webserv
CC = clang++
CFLAGS = -fsanitize=address -Wall -Wextra -Werror
INCLUDES = 
SRCS = tcp_client.cpp 

all : $(NAME)

$(NAME):
	$(CC)  $(CFLAGS) $(SRCS) $(Includes) -o $(NAME)

clean :

fclean: clean
	@(rm -rf $(NAME))
	@rm -rf *.dSYM

re:	fclean all

.PHONY : clean fclean re
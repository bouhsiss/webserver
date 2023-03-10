NAME = webserv
CC = clang++
CFLAGS = -Wall -Wextra -Werror
INCLUDES = 
SRCS = 

all : $(NAME)

$(NAME):
	$(CC)  $(CFLAGS) $(SRCS) $(Includes) -o $(NAME)

clean :

fclean: clean
	@(rm -rf $(NAME))
	@rm -rf *.dSYM

re:	fclean all

.PHONY : clean fclean re
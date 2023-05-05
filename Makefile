NAME = webserv
CC = c++
CFLAGS = -fsanitize=address #-Wall -Wextra -Werror
INCLUDES = 
SRCS = Configuration.cpp HttpMessage.cpp Location.cpp Server.cpp Http.cpp  main.cpp ServerFarm.cpp Exception.cpp

all : $(NAME)

$(NAME):
	$(CC)  $(CFLAGS) $(SRCS) $(Includes) -o $(NAME)

clean :

fclean: clean
	@(rm -rf $(NAME))
	@rm -rf *.dSYM

re:	fclean all

.PHONY : clean fclean re
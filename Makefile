NAME = webserv
CC = clang++
CFLAGS = -fsanitize=address #-Wall -Wextra -Werror
INCLUDES = 
SRCS = Configuration.cpp HttpMessage.cpp Location.cpp Request.cpp Server.cpp Http.cpp  main.cpp ServerFarm.cpp

all : $(NAME)

$(NAME):
	$(CC)  $(CFLAGS) $(SRCS) $(Includes) -o $(NAME)

clean :

fclean: clean
	@(rm -rf $(NAME))
	@rm -rf *.dSYM

re:	fclean all

.PHONY : clean fclean re
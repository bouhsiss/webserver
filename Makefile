NAME = webserv
CC = c++
CFLAGS = -fsanitize=address -Wall -Wextra -Werror
INCLUDES = 
SRCS = Configuration.cpp HttpMessage.cpp Location.cpp Server.cpp Http.cpp  main.cpp ServerFarm.cpp 
SRCS = Configuration.cpp HttpMessage.cpp Location.cpp Server.cpp Http.cpp  main.cpp ServerFarm.cpp

all : $(NAME)

$(NAME):
	$(CC)  $(CFLAGS) $(SRCS) $(Includes) -o $(NAME)

clean :

fclean: clean
	@(rm -rf $(NAME))
	@rm -rf *.dSYM
	@rm -rf .DS_Store

re:	fclean all

.PHONY : clean fclean re
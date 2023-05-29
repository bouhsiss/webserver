NAME = webserv
CC = c++
CFLAGS =  -Wall -Wextra -Werror #-fsanitize=address
INCLUDES = -I./config -I./http -I./HttpMessage -I./server -I./server/Location
SRCS = ./config/Configuration.cpp ./httpMessage/HttpMessage.cpp ./server/location/Location.cpp ./server/Server.cpp ./http/Http.cpp  main.cpp ./http/ServerFarm.cpp ./httpMessage/Request.cpp ./httpMessage/Response.cpp

all : $(NAME)

$(NAME):
	$(CC)  $(CFLAGS) $(SRCS) $(INCLUDES) -o $(NAME)

clean :

fclean: clean
	@(rm -rf $(NAME))
	@rm -rf *.dSYM
	@rm -rf .DS_Store

re:	fclean all

.PHONY : clean fclean re
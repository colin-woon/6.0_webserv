NAME        = network-test

CC          = c++
CFLAGS      = -Wextra -Wall -Werror -std=c++98 -g
RM          = rm -rf

SRCS        = main.cpp		Sockets/Sockets.cpp		ServerLoop/ServerLoop.cpp

OBJS        = $(SRCS:.cpp=.o)

GREEN       = \033[32m
RED         = \033[31m
RESET       = \033[0m

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)$(NAME) executable compiled!$(RESET)"

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)
	@echo "$(RED)Object (*.o) files removed!$(RESET)"

fclean: clean
	$(RM) $(NAME)
	@echo "$(RED)Executables removed!$(RESET)"

re: fclean all

.PHONY: all clean fclean re


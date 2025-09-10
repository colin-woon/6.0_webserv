NAME = webserv
SRCS = srcs/main.cpp\
	srcs/parsing/Parsing.cpp srcs/parsing/Token.cpp srcs/parsing/Server.cpp srcs/parsing/Location.cpp\
	srcs/networking/Sockets/Sockets.cpp srcs/networking/ServerLoop/ServerLoop.cpp\
	srcs/networking/ServerLoop/PollHelpers.cpp srcs/networking/ServerLoop/ServerHelpers.cpp srcs/networking/ServerLoop/Timeout.cpp
OBJS = $(SRCS:.cpp=.o)

CXX = c++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror
RM = rm -rf

GREEN = \033[0;32m
RED = \033[0;31m
RESET = \033[0m

%.o: %.cpp
		@$(CXX) $(CXXFLAGS) -c $< -o $@
		@echo "$(GREEN)creating object files....$(RESET)"

all: $(NAME)

$(NAME): $(OBJS)
		@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
		@echo "$(GREEN)$(NAME) created$(RESET)"

clean:
		@rm -rf $(OBJS) > /dev/null 2>&1
		@echo "$(RED)object files were deleted$(RESET)"

fclean: clean
		@rm -rf $(NAME) > /dev/null 2>&1
		@echo "$(RED)$(NAME) was deleted$(RESET)"

re: fclean all

.PHONY: all clean fclean re
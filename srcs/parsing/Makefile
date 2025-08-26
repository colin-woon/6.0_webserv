NAME = webserv
SRCS = main.cpp parsing.cpp token.cpp server.cpp location.cpp
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
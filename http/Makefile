#------------------------------------------------------------------------------#
#                                  GENERICS                                    #
#------------------------------------------------------------------------------#

.PHONY: all clean fclean re
# .SILENT:

#------------------------------------------------------------------------------#
#                                VARIABLES                                     #
#------------------------------------------------------------------------------#

# add back WFLAGS later to CFLAGS, removed cause it doesnt work for testing
# Compiler and flags
CXX			=	c++
CFLAGS		=	$(INCLUDES) $(STANDARD) $(DEBUG)
STANDARD	=	-Wpedantic -std=c++98
WFLAGS		=	-Wall -Werror -Wextra
INCLUDES	=	-I$(INC_DIR)
DEBUG		=	-g3
FSAN		=	-fsanitize=address,leak
RM			=	rm -rf

# Output file name
NAME	=	webserv

# Directories
INC_DIR			=	includes/
SRCS_DIR		=	srcs/
OBJS_DIR		=	bin/

INC_FILES		=
SRCS_FILES		=	srcs/test.cpp
OBJS_FILES		=	$(patsubst $(SRCS_DIR)%.c, $(OBJS_DIR)%.o, $(SRCS_FILES))

#------------------------------------------------------------------------------#
#                                 TARGETS                                      #
#------------------------------------------------------------------------------#

all: $(NAME)

# Generates output file
$(NAME): $(OBJS_FILES)
	$(CXX) $(CFLAGS) $(OBJS_FILES) -o $(NAME)

# Rule to compile the object files
$(OBJS_DIR)%.o: $(SRCS_DIR)%.c $(INC_DIR)cub3d.h | $(OBJS_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -c $< -o $@

# Rule to create the object directory if it doesn't exist
$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)

# Removes objects
clean:
	$(RM) $(OBJS_DIR)

# Removes objects and executables
fclean: clean
	$(RM) $(NAME)

# Removes objects and executables and remakes
re: fclean $(OBJS_DIR) all

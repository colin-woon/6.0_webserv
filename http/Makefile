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
CFLAGS		=	$(INCLUDES) $(STANDARD) $(DEBUG) $(FSAN)
STANDARD	=	-Wpedantic -std=c++98
WFLAGS		=	-Wall -Werror -Wextra
INCLUDES	=	-I$(INC_DIR) -I$(MODELS_DIR)
DEBUG		=	-g3 -O0 -fstandalone-debug
FSAN		=	-fsanitize=address,leak
RM			=	rm -rf

# Output file name
NAME	=	webserv

# Directories
INC_DIR			=	includes/
MODELS_DIR		=	models/
SRCS_DIR		=	srcs/
OBJS_DIR		=	bin/

# Set search path for source files
VPATH			=	$(SRCS_DIR):$(MODELS_DIR)

HTTP_HEADERS	=	models/HttpRequest.hpp \
					models/HttpResponse.hpp \
					models/HttpHandlerGET.hpp \
					models/HttpHandler.hpp \
					models/HttpStatusCodes.hpp \

HTTP_MODELS		=	models/HttpRequest.cpp \
					models/HttpResponse.cpp \
					models/HttpHandlerGET.cpp \
					models/HttpHandler.cpp \

MODEL_FILES		=	$(HTTP_MODELS)
MODEL_HEADERS	=	$(HTTP_HEADERS)

INC_FILES		=
SRCS_FILES		=	srcs/test.cpp

# All source files combined
ALL_SRCS		=	$(SRCS_FILES) $(MODEL_FILES)

OBJS_FILES		=	$(addprefix $(OBJS_DIR), $(notdir $(ALL_SRCS:.cpp=.o)))
#------------------------------------------------------------------------------#
#                                 TARGETS                                      #
#------------------------------------------------------------------------------#

all: $(NAME)

# Generates output file
$(NAME): $(OBJS_FILES)
	$(CXX) $(CFLAGS) $(OBJS_FILES) -o $(NAME)

# Rule to compile the object files
$(OBJS_DIR)%.o: %.cpp $(MODEL_FILES) | $(OBJS_DIR)
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
re: fclean all

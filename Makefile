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
CFLAGS		=	$(INCLUDES) $(DEBUG) $(STANDARD) $(FSAN)
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
MODELS_DIR		=	models
SRCS_DIR		=	srcs/
OBJS_DIR		=	bin/

# Set search path for source files
VPATH			=	$(SRCS_DIR):$(MODELS_DIR)/http:$(MODELS_DIR)/networking/ServerLoop:$(MODELS_DIR)/networking/Sockets:$(MODELS_DIR)/parsing


PARSING_HEADERS =	models/parsing/Header.hpp \
					models/parsing/Location.hpp \
					models/parsing/Parsing.hpp \
					models/parsing/Server.hpp \
					models/parsing/Token.hpp \
					models/parsing/Webserv.hpp \

PARSING_MODELS =	models/parsing/Location.cpp \
					models/parsing/Parsing.cpp \
					models/parsing/Server.cpp \
					models/parsing/Token.cpp \


NETWORKING_HEADERS =	models/networking/ServerLoop/PollHelpers.hpp \
						models/networking/ServerLoop/ServerHelpers.hpp \
						models/networking/ServerLoop/ServerLoop.hpp \
						models/networking/ServerLoop/Timeout.hpp \
						models/networking/ServerLoop/VhostHelpers.hpp \
						models/networking/Sockets/Sockets.hpp \

NETWORKING_MODELS = 	models/networking/ServerLoop/ServerHelpers.cpp \
						models/networking/ServerLoop/ServerLoop.cpp \
						models/networking/ServerLoop/Timeout.cpp \
						models/networking/Sockets/Sockets.cpp \
						models/networking/ServerLoop/PollHelpers.cpp \
						models/networking/ServerLoop/VhostHelpers.cpp \


HTTP_HEADERS	=	models/http/HttpRequest.hpp \
					models/http/HttpResponse.hpp \
					models/http/HttpHandlerGET.hpp \
					models/http/HttpHandlerPOST.hpp \
					models/http/HttpHandlerDELETE.hpp \
					models/http/HttpHandler.hpp \
					models/http/HttpRequestParser.hpp \
					models/http/HttpExceptions.hpp \
					models/http/HttpUtils.hpp \
					models/http/FileHandler.hpp \
					models/http/Router.hpp \

HTTP_MODELS		=	models/http/HttpRequest.cpp \
					models/http/HttpResponse.cpp \
					models/http/HttpHandlerGET.cpp \
					models/http/HttpHandlerPOST.cpp \
					models/http/HttpHandlerDELETE.cpp \
					models/http/HttpHandler.cpp \
					models/http/HttpUtils.cpp \
					models/http/HttpRequestParser.cpp \
					models/http/HttpExceptions.cpp \
					models/http/FileHandler.cpp \
					models/http/Router.cpp \

MODEL_FILES		=	$(HTTP_MODELS) $(NETWORKING_MODELS) $(PARSING_MODELS)
MODEL_HEADERS	=	$(HTTP_HEADERS) $(NETWORKING_HEADERS) $(PARSING_HEADERS)

INC_FILES		=
SRCS_FILES		=	srcs/main.cpp

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
$(OBJS_DIR)%.o: %.cpp | $(OBJS_DIR)
	mkdir -p $(dir $@)
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

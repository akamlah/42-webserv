# **********************************************************************#
#																		#
#								webserv									#
#																		#
# ********************************************************************* #

EXECUTABLE_NAME = webserv

# ********************************************************************* #

CC = clang++

CFLAGS = \
	-Wall -Wextra -Werror \
	-std=c++98 \
	-Wc++98-compat \
	-pedantic

# ********************************************************************* #
# project-specific (setup):

ENTRY_POINT = webserv.cpp

SOURCES_DIR = ./src/
INCLUDE_DIR = ./include/

CLASSES_NAME = \
	Server \
	Socket \
	Request \
	Response \
	Config \
	Poll \
	CGI \
	Connection 
	
OTHER_CPPFILES = \

INTERFACES = \

HPP_TEMPLATES = \

OTHER_HEADERS = \
	utility.hpp \
	http_type_traits.hpp

# ********************************************************************* #
# internal setup:
# (Classes must have .cpp/.hpp extension to be compiled)

CLASS_CPP := $(addsuffix .cpp, $(CLASSES_NAME))
CLASS_HPP := $(addsuffix .hpp, $(CLASSES_NAME))
INTERFACES_HPP := $(addsuffix .hpp, $(INTERFACES))

SOURCES_LIST = $(CLASS_CPP) $(ENTRY_POINT) $(OTHER_CPPFILES)
SOURCES = $(addprefix $(SOURCES_DIR), $(SOURCES_LIST))

INCLUDE_DEPS_LIST = $(CLASS_HPP) $(HPP_TEMPLATES) $(OTHER_HEADERS)
INCLUDE_DEPS = $(addprefix $(INCLUDE_DIR), $(INCLUDE_DEPS_LIST))

OBJ_DIR = ./obj/

OBJ_FILES = $(SOURCES_LIST:.cpp=.o)

OBJ = $(addprefix $(OBJ_DIR), $(OBJ_FILES))

$(OBJ_DIR)%.o: $(SOURCES_DIR)%.cpp $(INCLUDE_DEPS)
	@$(CC) $(CFLAGS)  -c $< -o $@ 

# ********************************************************************* #
# compile all:

all: $(EXECUTABLE_NAME)

$(EXECUTABLE_NAME): obj $(OBJ) $(INCLUDE_DEPS)
	@ echo "\033[38;5;36mBuilding webserver..."
	@ $(CC) $(CFLAGS) -o $(EXECUTABLE_NAME) $(OBJ)
	@ echo "Done ✓\033[0m"


# $(EXECUTABLE_NAME): $(SOURCES) $(INCLUDE_DEPS)
# 	@ echo "\033[38;5;36mBuilding webserver..."
# 	@ $(CC) $(CFLAGS) -o $(EXECUTABLE_NAME) $(SOURCES)
# 	@ echo "Done ✓\033[0m"

obj: 
	@mkdir -p $(OBJ_DIR)

# compile with fsanitize address flag: 'make dbg'

sani: re
	@ echo "\033[34;0;36mDebug mode: fsanitize address"
	@ $(CC) $(CFLAGS) -o $(EXECUTABLE_NAME) $(SOURCES) -g3 -fsanitize=address

dbg:
	@ echo "\033[34;0;36mDebug mode: flag DEBUG = true"
	@ $(CC) $(CFLAGS) -o $(EXECUTABLE_NAME) $(SOURCES) -DDEBUG=1

.PHONY = clean

clean:
	@ rm -rf $(EXECUTABLE_NAME)
	@ rm -f $(OBJ_DIR)/*

fclean: clean
	@ rm -f $(EXECUTABLE_NAME)
	@ rm -rf *.dSYM

re: fclean all


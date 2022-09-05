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
	Config \
	Response

OTHER_CPPFILES = \

INTERFACES = \

HPP_TEMPLATES = \

OTHER_HEADERS = \
	utility.hpp

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

# ********************************************************************* #
# compile all:

all: $(EXECUTABLE_NAME)

$(EXECUTABLE_NAME): $(SOURCES) $(INCLUDE_DEPS)
	@ echo "\033[38;5;36mBuilding webserver..."
	@ $(CC) $(CFLAGS) -o $(EXECUTABLE_NAME) $(SOURCES)
	@ echo "Done ✓\033[0m"

# compile with fsanitize address flag: 'make dbg'
dbg: re
	@ echo "\033[34;5;36mDebug mode: fsanitize address"
	@ $(CC) $(CFLAGS) -o $(EXECUTABLE_NAME) $(SOURCES) -g3 -fsanitize=address

.PHONY = clean

clean:
	@ rm -rf $(EXECUTABLE_NAME)

fclean: clean
	@ rm -f $(EXECUTABLE_NAME)
	@ rm -rf *.dSYM

re: fclean all


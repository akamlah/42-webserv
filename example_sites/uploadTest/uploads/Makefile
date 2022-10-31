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

CLASSES = \

OTHER_CPPFILES = \

INTERFACES = \

HPP_TEMPLATES = \

OTHER_HEADERS = \
	
# ********************************************************************* #
# internal setup:
# (Classes must have .cpp/.hpp extension to be compiled)

CLASS_CPP := $(addsuffix .cpp, $(CLASSES))
CLASS_HPP := $(addsuffix .hpp, $(CLASSES))
INTERFACES_HPP := $(addsuffix .hpp, $(INTERFACES))

SOURCES = $(CLASS_CPP) $(ENTRY_POINT) $(OTHER_CPPFILES)
HEADER_DEPS = $(CLASS_HPP) $(HPP_TEMPLATES) $(OTHER_HEADERS)

# ********************************************************************* #
# compile all:

all: $(EXECUTABLE_NAME)

$(EXECUTABLE_NAME): $(SOURCES) $(HEADER_DEPS)
	@ echo "\033[38;5;37mBuilding webserver..."
	@ $(CC) $(CFLAGS) -o $(EXECUTABLE_NAME) $(SOURCES)
	@ echo "Done ✓\033[0m"

.PHONY = clean

clean:
	@ rm -rf $(EXECUTABLE_NAME)

fclean: clean
	@ rm -f $(EXECUTABLE_NAME)
	@ rm -rf *.dSYM
	@ rm -rf

re: fclean all

dbg: re
	$(CC) $(CFLAGS) -o $(EXECUTABLE_NAME) $(SOURCES) -g3 -fsanitize=address

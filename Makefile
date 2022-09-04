NAME = webserv

CC = c++

FLAGS = -std=c++11 -Wall -Wextra #-Werror -O3

SRC_DIR = ./
OBJ_DIR = ./obj/
INC_DIR1 = ./src/
INC_DIR2 = ./src/base/
INC_DIR3 = ./src/iterators/
INC_DIR4 = ./src/utils/

SRC_FILES =  Socket.cpp ListenSocket.cpp main.cpp

OBJ_FILES = $(SRC_FILES:.cpp=.o)

SRC = $(addprefix $(SRC_DIR), $(SRC_FILES))
OBJ = $(addprefix $(OBJ_DIR), $(OBJ_FILES))

INC_FILES =  *.hpp

all : $(NAME)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(INC_FILES)
	@echo "updated $<"
	@$(CC) $(FLAGS) -I $(INC_DIR1) -I $(INC_DIR2) -I $(INC_DIR3) -I $(INC_DIR4) -o $@ -c $<

$(NAME) : obj $(OBJ)
	@echo ".o files updated!...linking..."
	@$(CC) $(FLAGS) $(OBJ) -o $(NAME)
	@echo "$(NAME) executable compiled!"

obj:
	@mkdir -p $(OBJ_DIR)

clean :
	cd $(OBJ_DIR) && rm -rf $(OBJ_FILES)
	rm ./*.o

fclean : clean
	rm -rf $(NAME)

re : fclean all
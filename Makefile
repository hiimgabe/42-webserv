RED		:= \033[0;31m
GREEN	:= \033[0;32m
END		:= \033[0m


NAME 	= webserv
SRC_DIR	= source/
INC		= include/
OBJ_DIR = bin/
SRCS	=	$(addprefix $(SRC_DIR), main.cpp \
									Log.cpp \
									SimpleSocket.cpp \
									BindingSocket.cpp \
									ListeningSocket.cpp \
									ConnectionSocket.cpp \
									Config.cpp \
									Request.cpp \
									BufferRequest.cpp \
									CGIRequest.cpp \
									Signal.cpp \
									Utils.cpp)

OBJ = $(addprefix $(OBJ_DIR), $(notdir $(SRCS:.cpp=.o)))

FLAGS	= -Wall -Wextra -Werror -std=c++98 -g #-fsanitize=address

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(OBJ_DIR)
	@c++ $(FLAGS) -c $< -o $@
	@echo "[${GREEN}compiling${END}]: $<"

all: $(NAME)

$(OBJ_DIR):
	@mkdir bin/
	@echo "${GREEN}bin created${END}"

$(NAME): $(OBJ)
	@c++ $(FLAGS) -I $(INC) $(OBJ) -o $(NAME)
	@echo "${GREEN}${NAME} executable created${END}"
	@echo "-------------------------------------------------------------"
	@echo "${RED}IMPORTANT${END} - For CGI handling you must specify in\n\
	location /cgi-location, both cgi_path and cgi_extension MUST correspond\n\
	to the index of the extension in the list.(e.g., if .py is the first\n\
	extension in cgi_extension then the appropriate py interpreter must\n\
	also be the first entry in cgi_path.) The .cgi extension is intended\n\
	for binaries that do not require an interpreter."
	@echo "---------------------------${GREEN}Usage${END}-----------------------------"
	@echo "1. Define the locations, methods, etc, in the configuration file."
	@echo "2. Ensure the configuration file is correctly formated."
	@echo "3. Start the server(e.g., ./webserv config/config.conf)"
	@echo "-------------------------------------------------------------"

clean:
	@rm -rf ${OBJ_DIR}
	@echo "${RED}cleaning objects${END}"

fclean: clean
	@rm -rf ${NAME}
	@rm -f upload/upload.txt
	@echo "${RED}${NAME} deleted${END}"

re: fclean all

.PHONY: all clean fclean re
NAME = cubeworld

CC = gcc

CFLAGS = -g -Wall -Wextra -Werror# -Ofast -pipe -mtune=native -march=native -flto=8

INCLUDES_PATH = -I include

SRCS_PATH = src/

SRCS_NAME = main.c \
	window.c \
	player.c \
	world.c \
	chunk.c \
	block.c \

SRCS = $(addprefix $(SRCS_PATH), $(SRCS_NAME))

OBJS_PATH = obj/

OBJS_NAME = $(SRCS_NAME:.c=.o)

OBJS = $(addprefix $(OBJS_PATH), $(OBJS_NAME))

LIBRARIES = -lm
LIBRARIES+= -lglfw
LIBRARIES+= -lGL
LIBRARIES+= -lpthread
LIBRARIES+= -lGLU

all: odir $(NAME)

$(NAME): $(OBJS)
	@echo " - Making $@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LIBRARIES)

$(OBJS_PATH)%.o: $(SRCS_PATH)%.c
	@echo " - Compiling $<"
	@$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES_PATH)

odir:
	@mkdir -p $(OBJS_PATH)

clean:
	@echo " - Cleaning objs"
	@rm -f $(OBJS)

fclean: clean
	@echo " - Cleaning lib"
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re odir

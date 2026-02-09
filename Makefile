TARGET = codexion
CC = cc
CFLAGS = -Wall -Wextra -Werror

SRCS = main.c src/parser/parse.c src/validation/validate.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

fclean:
	clean
	rm -f $(TARGET)

re: clean all

.PHONY:
	clean fclean re all

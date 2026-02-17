TARGET = codexion
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread
DIR = coders
SRCS = $(DIR)/main.c \
	   $(DIR)/coders.c \
	   $(DIR)/parser/parse.c \
	   $(DIR)/validation/validate.c \
	   $(DIR)/stop/stop.c \
	   $(DIR)/logs/log.c
	   
	
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

fclean:clean
		rm -f $(TARGET)

re: clean all

.PHONY:
	clean fclean re all

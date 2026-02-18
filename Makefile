# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/18 13:25:33 by alyousse          #+#    #+#              #
#    Updated: 2026/02/18 13:25:33 by alyousse         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

TARGET = codexion
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread
DIR = coders
SRCS = $(DIR)/main.c \
		$(DIR)/coders.c \
		$(DIR)/parser/parse.c \
		$(DIR)/validation/validate.c \
		$(DIR)/stop/stop.c \
		$(DIR)/logs/log.c \
		$(DIR)/dongles/dongles.c \
		$(DIR)/parser/init_simulator.c

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

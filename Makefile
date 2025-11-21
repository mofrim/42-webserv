# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/12/01 11:45:58 by fmaurer           #+#    #+#              #
#    Updated: 2025/11/20 15:08:47 by fmaurer          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME 		= webserv

# The directories..
#
# the magic vpath... by using this we will not have to specify in which
# sub-directory a source file is located. the '%.o' and '%.cpp' wildcards will
# find any file in any directory listed in the VPATH. The same holds true for
# the header files. This also applies to change-detection!
# TL;DR: it makes things pretty easy ;)
VPATH		= ./src ./include
OBJDIR	= obj
INC_DIR = ./include

# The files..
SRCS		= main.cpp Webserv.cpp
HDRS		= Webserv.hpp

OBJS		= $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

# The programs..
CPP			= c++

# compiler flags
CFLAGS	= -Wall -Werror -Wextra -std=c++98
IFLAGS	= -I $(INC_DIR)

all: $(NAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)

# will never try to compile HDR or any other prequesite specified after `%.cpp`.
# this is because the `%.o` and matched against all .c-files in the current dir.
$(OBJDIR)/%.o: %.cpp $(HDRS) | $(OBJDIR)
	$(CPP) $(IFLAGS) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CPP) $(IFLAGS) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(OBJDIR)

nodebug: CFLAGS += -DDEBUG=0
nodebug:
	$(CPP) $(CFLAGS) -o $(NAME) $(SRC) $(MAINSRC)

debug: CFLAGS += -g
debug:
	$(CPP) $(CFLAGS) -o $(NAME) $(SRC) $(MAINSRC)

bear: fclean
	bear -- make

run: nodebug
	./$(NAME)

run-debug: debug
	./$(NAME)

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re nodebug debug bear run run-debug

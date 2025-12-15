# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/12/01 11:45:58 by fmaurer           #+#    #+#              #
#    Updated: 2025/12/15 00:38:50 by fmaurer          ###   ########.fr        #
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
VPATH		= ./src ./inc
OBJDIR	= obj
INC_DIR = ./inc

# The files..
SRCS			= main.cpp Webserv.cpp Logger.cpp ServerCfg.cpp Config.cpp \
						ConfigParser.cpp utils.cpp Server.cpp Client.cpp ConManager.cpp \
						Epoll.cpp
HDRS			= Webserv.hpp Logger.hpp ServerCfg.hpp Config.hpp ConfigParser.hpp \
						utils.hpp Server.hpp Client.hpp ConManager.hpp Epoll.hpp

OBJS		= $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

# The programs..
CPP			= c++

# compiler flags
CFLAGS	= -Wall -Werror -Wextra -std=c++98
CFLAGS	+= -g
CFLAGS	+= -DLOGLEVEL=0
IFLAGS	= -I $(INC_DIR)

all: $(NAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)

# will never try to compile HDR or any other prequesite specified after `%.cpp`.
# this is because the `%.o` and matched against all .c-files in the current dir.
# !! Attention: Any misspelled filename in HDRS or SRCS will lead to `make` not
# functioning without any meaningful error msg !!
$(OBJDIR)/%.o: %.cpp $(HDRS) | $(OBJDIR)
	$(CPP) $(IFLAGS) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CPP) $(IFLAGS) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(OBJDIR)

bear: fclean
	bear -- make

run: $(NAME)
	./$(NAME)

valgrind: $(NAME)
	valgrind --track-fds=yes -- ./webserv

tests:
	@cd tests && ./update_tests.sh
	@make -C tests
	@tests/webserv_tests

tests-clean:
	@make -C tests fclean

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re bear run tests tests-clean

# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/12/01 11:45:58 by fmaurer           #+#    #+#              #
#    Updated: 2026/04/23 22:57:19 by fmaurer          ###   ########.fr        #
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
SRCS			= main.cpp Webserv.cpp Logger.cpp VServerCfg.cpp Config.cpp \
						ConfigParser.cpp utils.cpp VServer.cpp Client.cpp Epoll.cpp \
						Webserv_Utils.cpp VServer_Utils.cpp RequestHandler.cpp \
						Request.cpp Socket.cpp Route.cpp HttpStatus.cpp ReqParse.cpp \
						Response.cpp

HDRS			= Webserv.hpp Logger.hpp VServerCfg.hpp Config.hpp ConfigParser.hpp \
						utils.hpp VServer.hpp Client.hpp Epoll.hpp RequestHandler.hpp \
						Request.hpp Socket.hpp Route.hpp HttpStatus.hpp ReqParse.hpp \
						Response.hpp

OBJS		= $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

# The programs..
CPP			= c++

# compiler flags
CFLAGS	= -Wall -Werror -Wextra -std=c++98
CFLAGS	+= -g
IFLAGS	= -I $(INC_DIR)

ifeq ($(DBG),0)
	CFLAGS	+= -g -DLOGLEVEL=0
endif
ifeq ($(DBG),1)
	CFLAGS	+= -g -DLOGLEVEL=1
endif
ifeq ($(DBG),2)
	CFLAGS	+= -g -DLOGLEVEL=2
endif

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

debug: CFLAGS += -DLOGLEVEL=2
debug: $(SRCS)
	$(CPP) $(IFLAGS) $(CFLAGS) -o $(NAME) $^

debug-run: debug
	./webserv

val: $(NAME)
	valgrind --track-fds=yes -s -- ./webserv

tests:
	@cd tests && ./update_tests.sh
	@make -C tests
	@tests/webserv_tests

tests-clean:
	@make -C tests fclean

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re bear run tests tests-clean debug debug-run

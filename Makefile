# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/12/01 11:45:58 by fmaurer           #+#    #+#              #
#    Updated: 2026/06/25 13:13:11 by fmaurer          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME 		= webserv

# OMG! Make can do parallel builds out of the box! Normally the param is `-j
# [N]` where `N` is the number of CPUs to use for build. Without the N it simply
# uses the maximum available number of CPUs
#
# ... sadly causes problems with multi-step recipes like `re`
# MAKEFLAGS += -j $(shell echo "`grep -c processor /proc/cpuinfo` / 2" | bc)

# The directories..
#
# the magic vpath... by using this we will not have to specify in which
# sub-directory a source file is located. the '%.o' and '%.cpp' wildcards will
# find any file in any directory listed in the VPATH. The same holds true for
# the header files. This also applies to change-detection! TL;DR: it makes things pretty easy ;) 
VPATH		= ./src ./inc

OBJDIR	= obj
INC_DIR = ./inc

# The files..
SRCS	= main.cpp Webserv.cpp Logger.cpp VServerCfg.cpp \
				ConfigParser.cpp utils.cpp VServer.cpp Client.cpp Epoll.cpp \
				Webserv_Utils.cpp VServer_Utils.cpp RequestHandler.cpp \
				Request.cpp Socket.cpp Route.cpp Request_Parsing.cpp Response.cpp \
				WsrvLib.cpp URI.cpp ConfigParser_Tokenize.cpp \
				ConfigParser_Process.cpp ConfigParser_ParseTok.cpp RequestBody.cpp \
				Response_HandlePost.cpp Response_HandleCGI.cpp \
				Response_HandleCGI_Run.cpp

HDRS	= Webserv.hpp Logger.hpp VServerCfg.hpp ConfigParser.hpp \
				utils.hpp VServer.hpp Client.hpp Epoll.hpp RequestHandler.hpp \
				Request.hpp Socket.hpp Route.hpp Response.hpp WsrvLib.hpp URI.hpp \
				RequestBody.hpp

OBJS	= $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

# The programs..
CPP	= c++

# compiler flags
CFLAGS	= -Wall -Werror -Wextra -std=c++98
IFLAGS	= -I $(INC_DIR)

# some colors for the log msgs
GRN = \e[0;32m
RED = \e[1;31m
WHT = \e[1;37m
GRE = \e[37m
YLW = \e[1;93m
RST = \e[0m
MSGOPN = $(YLW)--(($(GRN)
MSGEND = $(YLW)))--$(RST)

HOST = $(shell hostname)
ifeq ($(findstring wolfsburg,$(HOST)), wolfsburg)
	ECHO = echo
	ECHON = echo -n
else
	ECHO = echo -e
	ECHON = echo -en
endif

# logmsg makefile functions
log_msg_start = @$(ECHO) "\n$(MSGOPN) $(1) $(MSGEND)"
log_msg_mid = @$(ECHO) "$(MSGOPN) $(1) $(MSGEND)"
log_msg_end = @$(ECHO) "$(MSGOPN) $(1) $(MSGEND)\n"
log_msg_single = @$(ECHO) "\n$(MSGOPN) $(1) $(MSGEND)\n"

# output coloring
output_color_grey = @$(ECHON) "$(GRE)"
output_colr_reset =  @$(ECHON) "$(RST)"

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
	$(call log_msg_mid,Compiling $<...)
	$(call output_color_grey)
	$(CPP) $(IFLAGS) $(CFLAGS) -c $< -o $@
	$(call output_color_reset)


$(NAME): $(OBJS)
	$(call log_msg_mid,Linking / Compiling webserv binary...)
	$(call output_color_grey)
	$(CPP) $(IFLAGS) $(CFLAGS) -o $@ $^
	$(call log_msg_end, Done!)

clean:
	rm -rf $(OBJDIR)

bear: fclean
	bear -- make

run: $(NAME)
	./$(NAME)

run1: fclean
	make DBG=1
	./$(NAME)

run2: fclean
	make DBG=2
	./$(NAME)

dbg0:
	make DBG=0

dbg0-re: fclean
	make DBG=0

dbg1:
	make DBG=1

dbg1-re: fclean
	make DBG=1

dbg2:
	make DBG=2

dbg2-re: fclean
	make DBG=2

debug: CFLAGS += -DLOGLEVEL=2
debug: $(SRCS)
	$(CPP) $(IFLAGS) $(CFLAGS) -o $(NAME) $^

debug-run: debug
	./webserv

val: $(NAME)
	valgrind --track-fds=yes -s -- ./webserv

valfull: $(NAME)
	valgrind --track-fds=yes -s --leak-check=full -- ./webserv

cpptests:
	@cd tests/cpp_tests && ./update_tests.sh
	@make -C tests/cpp_tests

cpptests-run: tests
	@cd tests/cpp_tests && ./update_tests.sh
	@make -C tests/cpp_tests run

cpptests-clean:
	@make -C tests/cpp_tests fclean

cpptests-bear:
	@bear -- make -C tests/cpp_tests

reqtests:
	@cd tests/req_tests && ./run_test.sh

cfgtests:
	@cd tests/cfg_tests && ./run_test.sh

allMyTests: cpptests-run reqtests cfgtests

siege:
	$(call log_msg_single,Running siege on webserv...)
	siege -b --time 20s -f tests/siegeList.txt

42tester:
	$(call log_msg_single,Starting 42tester...)
	@tests/run_42tester.sh

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

re-run: re run

.PHONY: all clean fclean re bear run cpptests cpptests-run cpptests-clean \
	cpptests-bear shelltests debug debug-run run1 run2 re-run dbg1 dbg2 dbg1-re \
	dbg2-re cfg_tests req_tests valfull val dbg0 dbg0-re 42tester allMyTests

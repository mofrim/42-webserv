#!/usr/bin/env bash

# nix-alien "/home/mofrim/c0de/42/CC/18-webserv/webserv/www/42-cgi_tester/cgi_tester" "/home/mofrim/c0de/42/CC/18-webserv/webserv/www/42-cgi_tester/YoupiBanane/youpi.bla" | tee 1>&2

# /usr/bin/env 1>&2

# nix-alien "/home/mofrim/c0de/42/CC/18-webserv/webserv/www/42-cgi_tester/cgi_tester" "/home/mofrim/c0de/42/CC/18-webserv/webserv/www/42-cgi_tester/YoupiBanane/youpi.bla" | tee /dev/stderr

nix-alien "/home/mofrim/c0de/42/CC/18-webserv/webserv/www/42tester/cgi_tester" "$(readlink -f $@)" | tee /home/mofrim/c0de/42/CC/18-webserv/cgi_tester.out

#!/usr/bin/env bash

# simple script for monitoring memory usage of webserv
# definitely TODO: make something graphical for this

#watch -n 0.1 ps -p $(pgrep webserv) -o rss,uss,vsz,cmd
watch -n 0.1 ps -p $(pgrep webserv) -o rss,vsz,cmd

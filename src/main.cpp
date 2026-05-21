/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:37:25 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/21 17:01:03 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Webserv.hpp"

#include <iostream>
#include <signal.h>
#include <string.h>

Webserv *g_webserv = NULL;

volatile sig_atomic_t g_killme = 0;

void signalHandler(int signum)
{
  (void)signum;
  g_killme = 1;
}

int main(int ac, char **av, char **envp)
{
  Webserv webserv(envp);
  g_webserv = &webserv;

  if (signal(SIGINT, signalHandler) == SIG_ERR) {
    Logger::logErr("setting signal handler failed");
    return 1;
  }

  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    Logger::logErr("Ignoring SIGPIPE did not work out as expected!");
    return 1;
  }

  if (ac > 2) {
    Logger::logErr("Too many args");
    return 1;
  }
  else if (ac == 2)
    try {
      webserv.readConfig(av[1]);
      webserv.run();
    } catch (const std::exception& e) {
      Logger::logErr("main", e.what());
    }
  else
    try {
      webserv.run();
    } catch (const std::exception& e) {
      std::cout << "e.what(): " << e.what() << std::endl;
    }

  return 0;
}

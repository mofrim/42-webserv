/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:37:25 by fmaurer           #+#    #+#             */
/*   Updated: 2026/01/09 10:53:14 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Webserv.hpp"

#include <iostream>
#include <signal.h>
#include <string.h>

Webserv *g_webserv = NULL;

void shutdownHandler(int signum)
{
	(void)signum;
	if (g_webserv) {
		g_webserv->shutdownWebserv();
	}
}

int main(int ac, char **av)
{
	Webserv webserv;
	g_webserv = &webserv;

	if (signal(SIGINT, shutdownHandler) == SIG_ERR) {
		Logger::log_err("setting signal handler failed");
		return (1);
	}

	if (ac > 2) {
		Logger::log_err("Too many args");
		return (1);
	}
	else if (ac == 2)
		try {
			webserv.getServersFromCfg(av[1]);
			webserv.run();
		} catch (const std::exception& e) {
			std::cout << "e.what(): " << e.what() << std::endl;
		}
	else
		try {
			webserv.run();
		} catch (const std::exception& e) {
			std::cout << "e.what(): " << e.what() << std::endl;
		}

	return (0);
}

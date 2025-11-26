/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:37:25 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/25 15:26:12 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Webserv.hpp"

#include <signal.h>

Webserv *g_webserv = NULL;

void shutdownHandler(int signum)
{
	(void)signum;
	if (g_webserv) {
		g_webserv->shutdown();
	}
}

int main(int ac, char **av)
{
	Config	cfg;
	Webserv webserv;
	g_webserv = &webserv;

	if (signal(SIGINT, shutdownHandler) == SIG_ERR) {
		Logger::log_err("signal handler error");
		return (1);
	}

	if (ac == 1)
		webserv.run(cfg);
	else if (ac == 2) {
		cfg.parseCfgFile(av[1]);
		webserv.run(cfg);
	}
	else {
		Logger::log_err("Too many args");
		return (1);
	}

	return (0);
}

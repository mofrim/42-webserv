/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:37:25 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/12 16:45:23 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Webserv.hpp"

#include <signal.h>
#include <string.h>

Webserv *g_webserv = NULL;

void shutdownHandler(int signum)
{
	(void)signum;
	if (g_webserv) {
		g_webserv->shutdown();
	}
}

// int signal_setup(void (*sig_handler)(int))
// {
// 	struct sigaction sig_act;
//
// 	memset(&sig_act, 0, sizeof(struct sigaction));
// 	sig_act.sa_handler = sig_handler;
// 	sig_act.sa_flags	 = 0;
// 	sigemptyset(&sig_act.sa_mask);
// 	if (sigaction(SIGINT, &sig_act, NULL) == -1) {
// 		Logger::log_err("setting signal handler failed");
// 		return (-1);
// 	}
// 	if (sigaction(SIGTERM, &sig_act, NULL) == -1) {
// 		Logger::log_err("setting signal handler failed");
// 		return (-1);
// 	}
// 	return (0);
// }

int main(int ac, char **av)
{
	Config	cfg;
	Webserv webserv;
	g_webserv = &webserv;

	if (signal(SIGINT, shutdownHandler) == SIG_ERR) {
		Logger::log_err("setting signal handler failed");
		return (1);
	}

	// if (signal_setup(shutdownHandler) != 0) {
	// 	Logger::log_err("setting signal handler failed");
	// 	return (1);
	// }

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

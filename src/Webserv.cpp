/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:36:43 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/25 12:33:19 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Webserv.hpp"

// FIXME: remove cause sleep is not an allowed function!
#include <unistd.h>

Webserv::Webserv(): _shutdown_server(false) {}

Webserv::Webserv(const Webserv& other) { (void)other; }

Webserv& Webserv::operator=(const Webserv& other)
{
	(void)other;
	return (*this);
}

Webserv::~Webserv() {}

void Webserv::shutdown()
{
	Logger::log_warn("shutting down webserv...");
	_shutdown_server = true;
}

void Webserv::run(const Config& cfg)
{
	_cfg = cfg;
	while (!_shutdown_server) {
		Logger::log_msg("webserv running!");
		sleep(1);
	}
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:35:29 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/26 09:20:23 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "Config.hpp"

#include <netinet/in.h>
#include <sys/socket.h>

// NOTE: Naming convention for private class-members / methods: underscore at
// the beginning. In later use this enables us to omit the `this->`. This saves
// a lot of typing while being clear enough as we are still hand-writing our
// code here ;)

class Webserv {
	private:
		bool	 _shutdown_server;
		Config _cfg;

	public:
		// OCF
		Webserv();
		Webserv(const Webserv& other);
		Webserv& operator=(const Webserv& other);
		~Webserv();

		void shutdown();
		void run(const Config& cfg);
};

#endif

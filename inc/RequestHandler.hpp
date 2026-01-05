/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 19:12:58 by fmaurer           #+#    #+#             */
/*   Updated: 2026/01/05 08:25:13 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "Request.hpp"

#include <stdexcept>
#include <unistd.h>
#include <vector>

// FIXME: for now this is the read buffer size. definitely coud be higher.
#define READ_BUFSIZE 4096

class Server;

class RequestHandler {
	private:
		Server							*_srv;
		std::vector<Request> _requests;

		RequestHandler(const RequestHandler& other);
		RequestHandler();

	public:
		RequestHandler(Server *srv);
		RequestHandler& operator=(const RequestHandler& other);
		~RequestHandler();

		int readRequest(int fd);
		int writeResponse(int fd);

		class ReqHandlerException: public std::runtime_error {
			public:
				ReqHandlerException(const std::string& msg);
		};
};
#endif

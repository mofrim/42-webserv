/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 19:12:58 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/18 21:23:44 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <unistd.h>

// FIXME: for now this is the read buffer size. definitely coud be higher.
#define READ_BUFSIZE 4096

class Server;

class RequestHandler {
	private:
		Server *_srv;

		RequestHandler(const RequestHandler& other);
		RequestHandler& operator=(const RequestHandler& other);
		RequestHandler();

		// utils
		bool _isTerminatedReq(char *buf, ssize_t bytes_read);

	public:
		RequestHandler(Server *srv);
		~RequestHandler();

		int readRequest(int fd);
		int writeResponse(int fd);
};
#endif

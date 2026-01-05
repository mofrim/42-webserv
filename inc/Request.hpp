/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:07 by fmaurer           #+#    #+#             */
/*   Updated: 2026/01/05 08:51:47 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "ServerCfg.hpp"

#include <string>

// The Request class.
//
// - at the end of parsing _statusCode will be set. if, f.ex., _statusCode in
// [400 - 499] we need to display one of the error pages
//
// NEXT: implement Response class... it should hold the response header, utility
// functions for setting date, server name, content-length, content-type (!!!!
// it could also be an image!?!?) _and_ the response itself! Also we need to
// pass the statusCode and somehow decide, taking the servers cfg into account,
// what error page to display.

class Request {
	private:
		const ServerCfg *_srvcfg;
		std::string			 _reqstr;
		void						 _parseRequest();
		std::string			 _response; // FIXME: leave this as a string ?! No! Can also
																// be a stream of bytes. Will be another class!
		uint16_t _statusCode;

		bool _isTerminatedReq();

	public:
		// TODO: decide which ctors we really use here!
		Request();
		Request(const Request& other);
		Request(const ServerCfg *scfg, const std::string& rstr);
		Request& operator=(const Request& other);
		~Request();

		std::string getResponse() const;
};

#endif

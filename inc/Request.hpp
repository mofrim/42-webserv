/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:07 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/20 01:16:10 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "ServerCfg.hpp"

#include <string>

class Request {
	private:
		const ServerCfg *_srvcfg;
		std::string			 _reqstr;
		void						 _parseRequest();
		std::string			 _response; // FIXME: for now, this is only a string

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

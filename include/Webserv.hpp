/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:35:29 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/21 14:51:56 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <stdexcept>

class Logger {
	public:
		void logMsg(const std::string& msg) { std::cout << msg << std::endl; };
};

class Webserv {
	private:
	public:
		// OCF
		Webserv();
		Webserv(const Webserv& other);
		Webserv& operator=(const Webserv& other);
		~Webserv();

		// defining exception runtime_error here because many errors we will
		// encounter here will be produced during runtime
		class MyException: public std::runtime_error {
			public:
				MyException(const std::string& msg);
		};

		class logger: public Logger {};
};

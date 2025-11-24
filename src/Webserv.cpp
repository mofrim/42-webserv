/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:36:43 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/21 14:47:07 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

Webserv::Webserv() {}

Webserv::Webserv(const Webserv& other) { (void)other; }

Webserv& Webserv::operator=(const Webserv& other)
{
	(void)other;
	return (*this);
}

Webserv::~Webserv() {}

Webserv::MyException::MyException(const std::string& msg):
	std::runtime_error(" MyException: " + msg)
{}

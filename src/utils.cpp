/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 10:03:57 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/12 18:06:27 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// TODO: add missing conversion functions here.

#include "utils.hpp"

#include <sstream>

std::string int2str(int n)
{
	std::ostringstream oss;
	oss << n;
	return (oss.str());
}

// our own inet_ntoa function as by subject.pdf we figured that we shouldn't use
// the std function. converts ip-addr from network byte order (big endian) to
// string.
std::string inaddrToStr(const struct in_addr& addr)
{
	std::string ret("");
	ret.append(int2str(addr.s_addr & 255) + "." +
			int2str((addr.s_addr >> 8) & 255) + "." +
			int2str((addr.s_addr >> 16) & 255) + "." +
			int2str((addr.s_addr >> 24) & 255));
	return (ret);
}

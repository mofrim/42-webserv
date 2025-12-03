/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 10:03:57 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/03 11:36:31 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// TODO: add missing conversion functions here.

#include "utils.hpp"

std::string lit2str(int n) {
  std::ostringstream oss;
  oss << n;
  return (oss.str());
}

// our own inet_ntoa function as by subject.pdf we figured that we shouldn't use
// the std function. converts ip-addr from network byte order (big endian) to
// string.
std::string inaddrToStr(const struct in_addr &addr) {
  std::string ret("");
  ret.append(lit2str(addr.s_addr & 255) + "." +
             lit2str((addr.s_addr >> 8) & 255) + "." +
             lit2str((addr.s_addr >> 16) & 255) + "." +
             lit2str((addr.s_addr >> 24) & 255));
  return (ret);
}

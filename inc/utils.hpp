/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 11:26:23 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/18 17:55:59 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "typesAndConstants.hpp"

#include <netinet/in.h>
#include <set>

str int2str(int n);
str inAddrToStr(const struct in_addr& addr);
str getAddrPortStr4(const struct sockaddr_in& addr);

int           setFdNonBlocking(int fd);
std::ostream& operator<<(std::ostream& os, std::set<u16> lst);

template <typename T>
str getSetStr(const std::set<T>& s)
{
  str ret("");
  if (s.size() == 0)
    return ret;
  ret += "{";
  for (typename std::set<T>::iterator it = s.begin(); it != s.end();)
    ret += int2str(*it) + (++it != s.end() ? ", " : "}");
  return ret;
}

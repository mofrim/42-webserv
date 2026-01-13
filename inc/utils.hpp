/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 11:26:23 by fmaurer           #+#    #+#             */
/*   Updated: 2026/01/13 15:36:59 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <netinet/in.h>
#include <string>

std::string int2str(int n);
std::string inAddrToStr(const struct in_addr& addr);
std::string getAddrPortStr4(const struct sockaddr_in& addr);

int setFdNonBlocking(int fd);

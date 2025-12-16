/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 11:26:23 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/16 12:27:53 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <netinet/in.h>
#include <string>

std::string int2str(int n);
std::string inaddrToStr(const struct in_addr& addr);
int					setFdNonBlocking(int fd);

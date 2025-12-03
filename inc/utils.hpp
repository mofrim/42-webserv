/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 11:26:23 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/03 11:27:04 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <netinet/in.h>
#include <sstream>
#include <string>

std::string lit2str(int n);

std::string inaddrToStr(const struct in_addr &addr);

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Route.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 16:41:56 by fmaurer           #+#    #+#             */
/*   Updated: 2026/01/09 16:51:03 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTE_HPP
#define ROUTE_HPP

// A Route must be able to store data for a config like this

class Route {
	private:
	public:
		Route();
		Route(const Route& other);
		Route& operator=(const Route& other);
		~Route();
};

#endif

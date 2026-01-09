/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Route.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 16:42:51 by fmaurer           #+#    #+#             */
/*   Updated: 2026/01/09 16:50:24 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Route.hpp"

// -- OCF --

Route::Route() {}

Route::Route(const Route& other) { (void)other; }

Route& Route::operator=(const Route& other)
{
	(void)other;
	return (*this);
}

Route::~Route() {}
// -- OCF end --

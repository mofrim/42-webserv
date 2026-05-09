/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_hdr-template.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 16:00:31 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/09 17:38:13 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef OK
#define OK true
#endif

#ifndef KO
#define KO false
#endif

// global result marker
extern bool g_GlobalResult;

#include <cassert>
#include <iostream>
#include <map>
#include <stdexcept>

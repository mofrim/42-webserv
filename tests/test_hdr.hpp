/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_hdr.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 16:00:31 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/23 18:57:15 by fmaurer          ###   ########.fr       */
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

#include "Client.hpp"
#include "Config.hpp"
#include "ConfigParser.hpp"
#include "Epoll.hpp"
#include "HttpStatus.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "RequestHandler.hpp"
#include "Route.hpp"
#include "Socket.hpp"
#include "VServer.hpp"
#include "VServerCfg.hpp"
#include "Webserv.hpp"
#include "typesAndConstants.hpp"
#include "utils.hpp"

#include <cassert>
#include <iostream>
#include <map>
#include <stdexcept>

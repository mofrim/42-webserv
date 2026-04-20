/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_hdr-template.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 16:00:31 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/17 21:25:30 by fmaurer          ###   ########.fr       */
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
#include "Client.hpp"
#include "Config.hpp"
#include "ConfigParser.hpp"
#include "Epoll.hpp"
#include "ErrPages.hpp"
#include "Logger.hpp"
#include "RequestHandler.hpp"
#include "Request.hpp"
#include "Route.hpp"
#include "Socket.hpp"
#include "typesAndConstants.hpp"
#include "utils.hpp"
#include "VServerCfg.hpp"
#include "VServer.hpp"
#include "Webserv.hpp"

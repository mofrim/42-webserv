/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   typesAndConstants.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 23:55:04 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/23 21:28:35 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <stdint.h>
#include <string>

#define VERSION "v0.4"

typedef std::string str;
typedef uint16_t    u16;
typedef uint32_t    u32;

#define MAX_BODY_SIZE 1000000

// default port for the default server
#define DEFAULT_PORT 4284

// default server name
#define DEFAULT_SRV_NAME "localhost"

// max num of clients.
#define MAX_CLIENTS 1000

#ifndef OK
#define OK 1
#endif

#ifndef KO
#define KO 0
#endif

#define CRLF "\r\n"

// just for explicitness sake define the possible return values of the
// requestHandler as constants here
enum { REQ_READ = 0, REQ_WRITE = 1, REQ_ERR = -1, REQ_INC = 2, REQ_DISCO = 3 };

typedef enum { M_GET, M_POST, M_DELETE, M_UNKNOWN } e_Method;

// the request-line
typedef struct {
    e_Method method;
    str      target;
    str      httpVersion;
} t_RequestLine;

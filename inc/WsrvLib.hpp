/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WsrvLib.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 17:31:03 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/01 09:46:09 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// All the consts. All the (mime-)types. All in one place.
// This is the place for everything that has no specific place or would lead to
// too small classes and too many files.

#pragma once

#include <map>
#include <set>
#include <stdint.h>
#include <string>

#define VERSION "v0.5"

// type abbreviations
typedef std::string str;
typedef uint16_t    u16;
typedef uint32_t    u32;

// maximum number of headers. nginx has this set to 1000 per default. above that
// -> HTTP_400
#define MAX_HEADER_LINES 500
#define MAX_BODY_SIZE 1000000
#define READ_BUFSIZE 4096
#define MAX_CLIENTS 1000
#define DEFAULT_SRV_NAME "localhost"
#define DEFAULT_PORT 1111

// return value convenience
#ifndef OK
#define OK 1
#endif

#ifndef KO
#define KO 0
#endif

#define CRLF "\r\n"
#define CRLFX2 "\r\n\r\n"

// using C++ enum def syntax. this equivalent to `typedef enum ...`
// C++11 would have scoped enums `enum class HTTPVersion { } ` which can then be
// accessed via f.ex. HTTPVersion::M_GET
enum e_Method { M_GET, M_POST, M_DELETE, M_UNKNOWN };

// use this struct to transport the values returned from Socket::bindSocket
typedef struct {
    str ip;
    str cname;
    int fd;
} t_AddrinfoReturn;

// will use this struct for storing ip, cname and port, i.e. all relevant info
// for one interface.
typedef struct {
    str                             ip;
    str                             cname;
    std::set< std::pair<u16, int> > portFd;
} t_vsrvInterface;

// ... and the comparison operator for that type
bool operator==(const t_vsrvInterface& i1, const t_vsrvInterface& i2);

// the request-line
typedef struct {
    str      target;
    str      httpVersion;
    e_Method method;
} t_RequestLine;

// Wow! This is actually a desired use-case for `namespace`. It allows to scope
// enums! Now they can be accessed via HTTPVersion::HHTP_1_0 and so on.
//
// namespace HTTPVersion {
// enum { HTTP_1_0, HTTP_1_1, UNKNOWN };
// };
//
// But i prefer having a type here...
enum e_HTTPVersion { HTTP_1_0, HTTP_1_1, UNKNOWN };

// For expressiveness' sake declare them here
enum e_HTTPStatus {
  HTTP_200 = 200,
  HTTP_300 = 300,
  HTTP_400 = 400,
  HTTP_404 = 404,
  HTTP_408 = 408,
  HTTP_500 = 500,
  HTTP_501 = 501,
  HTTP_502 = 502,
};

typedef struct {
    const u16 maxClients;
    const u16 connKeepaliveTimeout;
    const u16 reqTimeout;
} t_GlobalWsrvSettings;

class WsrvLib {
  private:
    WsrvLib();
    WsrvLib(const WsrvLib& other);
    WsrvLib& operator=(const WsrvLib& other);
    ~WsrvLib();

    static str _getTemplateWithErrStr(const str& s);

    static const std::map<str, str> _mimeTypes;
    static std::map<str, str>       _initMimeTypes();

    static const std::map<u16, str> _statusCodes;
    static std::map<u16, str>       _initStatusCodes();

    static t_GlobalWsrvSettings _initWsrvSettings();

  public:
    static str                        getDefaultErrPage(u16 code);
    static str                        getStatusStr(u16 code);
    static str                        getMimeTypeFromPath(const str& p);
    static const t_GlobalWsrvSettings WsrvSettings;
};

// class HTTPVersion {
//   private:
//     HTTPVersion();
//     ~HTTPVersion();
//
//   public:
//     const static u16 GET;
//     const static u16 POST;
//     const static u16 DELETE;
// };

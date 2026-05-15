/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WsrvLib.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 17:31:03 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/15 19:17:27 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// All the consts. All the (mime-)types. All in one place.
// This is the place for everything that has no specific place or would lead to
// too small classes and too many files.

#pragma once

#include "URL.hpp"

#include <map>
#include <set>
#include <stdint.h>
#include <string>

#define VERSION "v0.6"

extern bool g_WsrvTesting;

// type abbreviations
typedef std::string                 str;
typedef const std::string           constr;
typedef uint8_t                     u8;
typedef uint16_t                    u16;
typedef uint32_t                    u32;
typedef std::string::iterator       strIt;
typedef std::string::const_iterator constrIt;

// maximum number of headers. nginx has this set to 1000 per default. above that
// -> HTTP_400
#define MAX_HEADER_LINES 200
#define MAX_BODY_SIZE 1000000         // 1mb
#define MAX_CONTENT_LENGTH 5000000000 // 5gb

// https://datatracker.ietf.org/doc/html/rfc9112#section-3
#define MAX_REQLINE_LEN 8000
#define MAX_TARGET_LEN 7886

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

// Wow! This is actually a desired use-case for `namespace`. It allows to scope
// enums! Now they can be accessed via HTTPVersion::HHTP_1_0 and so on.
//
// namespace HTTPVersion {
// enum { HTTP_1_0, HTTP_1_1, UNKNOWN };
// };
//
// But i prefer having a type here...
enum e_HTTPVersion { HTTPVER_1_0, HTTPVER_1_1, HTTPVER_UNKNOWN };

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
    URL           target;
    e_HTTPVersion httpVersion;
    e_Method      method;
} t_RequestLine;

// For expressiveness' sake declare them here
enum e_HTTPStatus {
  HTTP_0   = 0,
  HTTP_200 = 200,
  HTTP_201 = 201,
  HTTP_300 = 300,
  HTTP_301 = 301,
  HTTP_302 = 302,
  HTTP_308 = 308,
  HTTP_400 = 400,
  HTTP_403 = 403,
  HTTP_404 = 404,
  HTTP_408 = 408,
  HTTP_413 = 413,
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

    static const std::map<str, str> _ext2MimeTypes;
    static std::map<str, str>       _initExt2MimeTypes();

    static const std::map<str, str> _mimeTypes2Ext;
    static std::map<str, str>       _initMimeTypes2Ext();

    static const std::map<u16, str> _statusCodes;
    static std::map<u16, str>       _initStatusCodes();

    static t_GlobalWsrvSettings _initWsrvSettings();

  public:
    static const t_GlobalWsrvSettings WsrvSettings;

    static str getDefaultStatusPage(e_HTTPStatus code, constr& opts = "");
    static str getStatusStr(e_HTTPStatus code);

    static str getMimeTypeFromPath(const str& p);
    static str getExtFromMimeType(constr& mime);

    static e_HTTPVersion str2HTTPVer(const str& s);
    static str           httpVer2Str(e_HTTPVersion v);
    static e_HTTPStatus  short2HttpStatus(u16 s);
    static e_HTTPStatus  str2HttpStatus(const str& s);
    static str           getAutoindex(constr& path, constr& route);
};

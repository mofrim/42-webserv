/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WsrvLib.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 17:40:43 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/15 23:12:06 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WsrvLib.hpp"
#include "utils.hpp"

// --------------------------------=[ OCF ]=-------------------------------- //

WsrvLib::WsrvLib() {}

WsrvLib::WsrvLib(const WsrvLib& other) { (void)other; }

WsrvLib& WsrvLib::operator=(const WsrvLib& other)
{
  (void)other;
  return (*this);
}

WsrvLib::~WsrvLib() {}

// ------------------------------=[ Methods ]=------------------------------ //

str WsrvLib::getDefaultStatusPage(e_HTTPStatus code, constr& opts)
{
  str ret =

      "<!DOCTYPE html>\n"
      "<html>\n"
      "<head>\n"
      "<title>WebServ Default Status Page</title>\n"
      "<style>\n"
      "body{"
      "margin: 100px 100px 0px 100px;"
      "text-align: center;"
      "}"
      ".msg{"
      "margin:100px;"
      "padding:6px;";

  ret += "background-color:" + str((code < HTTP_400 ? "green;" : "red;"));

  ret +=
      "border-radius:10px;"
      "border:4px;"
      "border-color:black;"
      "border-style:solid;"
      "}"
      "</style>\n"
      "</head>\n"
      "<body>\n"
      "<div class=\"msg\">\n"
      "<h1>\n";

  ret += getStatusStr(code) + "\n</h1>\n";

  if (!opts.empty())
    ret += "<h2><a href=\"" + opts + "\">" + opts + "</a></h2>\n";

  ret +=
      "</div>\n"
      "<hr>\n"
      "<p>m0fr1m's webserv</p>\n"
      "</body>\n"
      "</html>\n";

  return ret;
}

// -------------------=[ initialize relevant data maps ]=------------------- //

// others might have put this into files...

std::map<str, str> WsrvLib::_initExt2MimeTypes()
{
  std::map<str, str> m;

  m["bmp"]         = "image/bmp";
  m["css"]         = "text/css";
  m["csv"]         = "text/csv";
  m["gif"]         = "image/gif";
  m["html"]        = "text/html";
  m["ico"]         = "image/png";
  m["ics"]         = "text/calendar";
  m["jpeg"]        = "image/jpeg";
  m["jpg"]         = "image/jpeg";
  m["js"]          = "text/javascript";
  m["json"]        = "application/json";
  m["mp3"]         = "audio/mpeg";
  m["mp4"]         = "video/mp4";
  m["ogg"]         = "audio/ogg";
  m["otf"]         = "font/otf";
  m["pdf"]         = "application/pdf";
  m["png"]         = "image/png";
  m["php"]         = "application/x-httpd-php";
  m["svg"]         = "image/svg+xml";
  m["ttf"]         = "font/ttf";
  m["txt"]         = "text/plain";
  m["wav"]         = "audio/wav";
  m["woff"]        = "font/woff";
  m["woff2"]       = "font/woff2";
  m["webmanifest"] = "application/json";
  m["xhtml"]       = "application/xhtml+xml";
  m["xml"]         = "application/xml";
  m["dat"]         = "application/octet-stream";

  return m;
}

const std::map<str, str> WsrvLib::_ext2MimeTypes = _initExt2MimeTypes();

std::map<str, str> WsrvLib::_initMimeTypes2Ext()
{
  std::map<str, str> m;

  m["image/bmp"]                = "bmp";
  m["text/css"]                 = "css";
  m["text/csv"]                 = "csv";
  m["image/gif"]                = "gif";
  m["text/html"]                = "html";
  m["image/png"]                = "ico";
  m["text/calendar"]            = "ics";
  m["image/jpeg"]               = "jpeg";
  m["image/jpeg"]               = "jpg";
  m["text/javascript"]          = "js";
  m["application/json"]         = "json";
  m["audio/mpeg"]               = "mp3";
  m["video/mp4"]                = "mp4";
  m["audio/ogg"]                = "ogg";
  m["font/otf"]                 = "otf";
  m["application/pdf"]          = "pdf";
  m["image/png"]                = "png";
  m["application/x-httpd-php"]  = "php";
  m["image/svg+xml"]            = "svg";
  m["font/ttf"]                 = "ttf";
  m["text/plain"]               = "txt";
  m["audio/wav"]                = "wav";
  m["font/woff"]                = "woff";
  m["font/woff2"]               = "woff2";
  m["application/json"]         = "webmanifest";
  m["application/xhtml+xml"]    = "xhtml";
  m["application/xml"]          = "xml";
  m["application/octet-stream"] = "dat";

  return m;
}

const std::map<str, str> WsrvLib::_mimeTypes2Ext = _initMimeTypes2Ext();

// @brief Extracts the file-ext from the path-string and looks up the
//        corresponding mimestr in mimetype database
// @return The mimetype-string belonging to a file-extension. If ext not known
//         returns "text/plain" bc this also what nginx does.
str WsrvLib::getMimeTypeFromPath(const str& p)
{
  str ext = p.substr(p.rfind(".") + 1);

  std::map<str, str>::const_iterator it = _ext2MimeTypes.find(ext);
  if (it != _ext2MimeTypes.end())
    return it->second;

  return "application/octet-stream";
}

str WsrvLib::getExtFromMimeType(constr& mime)
{
  std::map<str, str>::const_iterator it = _mimeTypes2Ext.find(mime);
  if (it != _mimeTypes2Ext.end())
    return it->second;
  return "dat";
}

std::map<u16, str> WsrvLib::_initStatusCodes()
{
  std::map<u16, str> s;
  s[200] = "OK";
  s[201] = "Created";
  s[204] = "No Content";
  s[301] = "Moved Permanently";
  s[302] = "Found";
  s[303] = "See Other";
  s[307] = "Temporary Redirect";
  s[308] = "Permanent Redirect";
  s[400] = "Bad Request";
  s[401] = "Unauthorized";
  s[403] = "Forbidden";
  s[404] = "Not Found";
  s[405] = "Not Allowed";
  s[408] = "Request Timeout";
  s[413] = "Content Too Large";
  s[415] = "Unsupported Media Type";
  s[500] = "Internal Server Error";
  s[503] = "Service Unavailable";
  s[504] = "Gateway Timeout";
  s[505] = "HTTP Version Not Supported";

  return s;
}

const std::map<u16, str> WsrvLib::_statusCodes = _initStatusCodes();

str WsrvLib::getStatusStr(e_HTTPStatus code)
{
  std::map<u16, str>::const_iterator it = _statusCodes.find(code);
  if (it != _statusCodes.end())
    return int2str(code) + " " + it->second;
  return int2str(code) + " Unknown HTTP Status Code!";
}

t_GlobalWsrvSettings WsrvLib::_initWsrvSettings()
{
  t_GlobalWsrvSettings s = {
      .maxClients = 1000, .connKeepaliveTimeout = 60, .reqTimeout = 5};

  return s;
}

const t_GlobalWsrvSettings WsrvLib::WsrvSettings = _initWsrvSettings();

// two vsrvInterfaces are the same iff their ip and cname match
bool operator==(const t_vsrvInterface& i1, const t_vsrvInterface& i2)
{
  if (i1.ip != i2.ip || i1.cname != i2.cname)
    return false;
  return false;
}

e_HTTPVersion WsrvLib::str2HTTPVer(const str& s)
{
  if (s == "HTTP/1.1")
    return HTTPVER_1_1;
  if (s == "HTTP/1.0")
    return HTTPVER_1_0;
  return HTTPVER_UNKNOWN;
}

str WsrvLib::httpVer2Str(e_HTTPVersion v)
{
  switch (v) {
    case HTTPVER_1_1:
      return "HTTP/1.1";
    case HTTPVER_1_0:
      return "HTTP/1.0";
    default:
      return "HTTP/*";
  }
}

e_HTTPStatus WsrvLib::short2HttpStatus(u16 s)
{
  switch (s) {
    case 200:
      return HTTP_200;
    case 300:
      return HTTP_300;
    case 301:
      return HTTP_301;
    case 302:
      return HTTP_302;
    case 308:
      return HTTP_308;
    case 400:
      return HTTP_400;
    case 404:
      return HTTP_404;
    case 408:
      return HTTP_408;
    case 413:
      return HTTP_413;
    case 500:
      return HTTP_500;
    case 501:
      return HTTP_501;
    case 502:
      return HTTP_502;
    default:
      return HTTP_0;
  }
}

e_HTTPStatus WsrvLib::str2HttpStatus(const str& s)
{
  u16 code = str2u16(s);
  return short2HttpStatus(code);
}

// global flag used to activate certain debug output.
bool g_WsrvTesting = false;

// return a nice autoindex page.
str WsrvLib::getAutoindex(constr& path, constr& route)
{
  std::set<str> files = listDirFiles(path, true);
  if (files.empty())
    return "";

  str slash;
  if (!route.empty() && route[route.length() - 1] != '/')
    slash = "/";

  str ret =
      "<!DOCTYPE html>\n"
      "<html>\n"
      "<head>\n"
      "<title>Autoindex</title>\n"
      "<style>"
      "html,body{height:100%;}"
      "body{"
      "background:"
      "radial-gradient(circle at 20% 20%, rgba(14,165,233,0.35), transparent "
      "30%),"
      "radial-gradient(circle at 80% 30%, rgba(168,85,247,0.30), transparent "
      "30%),"
      "radial-gradient(circle at 50% 80%, rgba(59, 130, 246, 0.25), "
      "transparent 35%),"
      "linear-gradient(135deg, #020617 0%, #0f172a 40%, #111827 100%);"
      "color: #fff;"
      "background-attachment: fixed;"
      "font-family: Fantasy, Arial, Helvetica, sans-serif;"
      "margin: 100px 100px 0px 100px;"
      "text-align: left;"
      "}"
      ".msg{"
      "margin:50px 100px 50px 100px;"
      "padding:12px;"
      "background:rgba(255, 255, 255, 0.35);"
      "backdrop-filter:blur(16px);"
      "border-radius:10px;"
      "border:4px;"
      "border-color:rgba(255, 255, 255, 0.3);"
      "border-style:solid;"
      "}"
      "p{text-align: center; font-size: 1.2rem; font-weight: 700; margin: "
      "4px;}"
      "a {color:rgba(24, 44, 79, 1.0);transition:filter 0.3s ease;}"
      "a#dir{color:rgba(55, 40, 94, 1.0);}"
      "a:hover{filter: brightness(2.0);}"
      "</style>\n"
      "</head>\n"
      "<body>\n"
      "<h1 style=\"text-align: center;\"> Autoindex of '" +
      route + "'</h1>";

  ret += "<div class=\"msg\">\n";

  for (std::set<str>::iterator it = files.begin(); it != files.end(); ++it) {
    str id;
    if (!it->empty() && (*it)[it->size() - 1] == '/')
      id = " id=\"dir\"";
    ret += "<p><a href=\"" + str(route + slash + *it) + "\"" + id + ">" + *it +
        "</a></p>\n";
  }
  ret +=
      "</div>\n"
      "<hr>\n"
      "<p>mofrim's WebServ</p>\n"
      "</body>\n"
      "</html>\n";

  return ret;
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WsrvLib.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 17:40:43 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/26 16:01:11 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WsrvLib.hpp"
#include "utils.hpp"

// --------------------------------=[ OCF ]=-------------------------------- //

WsrvLib::WsrvLib()
{}

WsrvLib::WsrvLib(const WsrvLib& other)
{
  (void)other;
}

WsrvLib& WsrvLib::operator=(const WsrvLib& other)
{
  (void)other;
  return (*this);
}

WsrvLib::~WsrvLib()
{}

// ------------------------------=[ Methods ]=------------------------------ //

str WsrvLib::_getTemplateWithErrStr(const str& s)
{
  str ret =
      "<html>\n"
      "<head>\n"
      "<title>m0fr1m's Webserv Error Page</title>\n"
      "<style>\n"
      "body {\n"
      "margin: 100px 100px 0px 100px;\n"
      "text-align: center;\n"
      "}\n"
      ".msg {\n"
      "margin: 100px;\n"
      "padding: 6px;\n"
      "background-color: red;\n"
      "border-radius: 10px;\n"
      "border: 4px;\n"
      "border-color: black;\n"
      "border-style: solid;\n"
      "}\n"
      "</style>\n"
      "</head>\n"
      "<body>\n"
      "<div class=\"msg\">\n"
      "<h1>\n";

  ret += s +
      "\n</h1>\n"
      "</div>\n"
      "<hr>\n"
      "<p>m0fr1m's webserv</p>\n"
      "</body>\n"
      "</html>\n";

  return ret;
}

str WsrvLib::getDefaultErrPage(u16 code)
{
  return _getTemplateWithErrStr(getStatusStr(code));
}

// -------------------=[ initialize relevant data maps ]=------------------- //

// others might have put this into files...

std::map<str, str> WsrvLib::_initMimeTypes()
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

  return m;
}

const std::map<str, str> WsrvLib::_mimeTypes = _initMimeTypes();

// @brief Extracts the file-ext from the path-string and looks up the
//        corresponding mimestr in mimetype database
// @return The mimetype-string belonging to a file-extension. If ext not known
//         returns "text/plain" bc this also what nginx does.
str WsrvLib::getMimeTypeFromPath(const str& p)
{
  str ext = p.substr(p.rfind(".") + 1);

  std::map<str, str>::const_iterator it = _mimeTypes.find(ext);
  if (it != _mimeTypes.end())
    return it->second;

  return "text/plain";
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
  s[413] = "Payload Too Large";
  s[415] = "Unsupported Media Type";
  s[500] = "Internal Server Error";
  s[503] = "Service Unavailable";
  s[504] = "Gateway Timeout";
  s[505] = "HTTP Version Not Supported";

  return s;
}

const std::map<u16, str> WsrvLib::_statusCodes = _initStatusCodes();

str WsrvLib::getStatusStr(u16 code)
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

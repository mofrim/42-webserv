/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WrsvLib.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 17:40:43 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/24 17:41:36 by fmaurer          ###   ########.fr       */
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
//

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

// returns the corresponding HTML status-code string for a given status code.
// also for usage in response generation
str WsrvLib::getStatusStr(u16 code)
{
  str errstr;
  switch (code) {
  case 200:
    errstr = "200 OK";
    break;
  case 400:
    errstr = "400 Bad Request";
    break;
  case 403:
    errstr = "403 Forbidden";
    break;
  case 404:
    errstr = "404 Page Not Found";
    break;
  case 405:
    errstr = "404 Method Not Allowed";
    break;
  case 408:
    errstr = "408 Request Timeout";
    break;
  case 413:
    errstr = "413 Content Too Large";
    break;
  case 418:
    errstr = "418 I'm a teapot";
    break;
  case 500:
    errstr = "500 Internal Server Error";
    break;
  case 501:
    errstr = "501 Not Implemented";
    break;
  case 502:
    errstr = "502 Bad Gateway";
    break;
  default:
    errstr += int2str(code) + " - Unknown HTML Status Code!";
  }
  return errstr;
}

str WsrvLib::getDefaultErrPage(u16 code)
{
  return _getTemplateWithErrStr(getStatusStr(code));
}

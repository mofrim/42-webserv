/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrPages.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 16:51:47 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/20 17:21:13 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ErrPages.hpp"
#include "utils.hpp"

// --------------------------------=[ OCF ]=-------------------------------- //

ErrPages::ErrPages()
{}

ErrPages::ErrPages(const ErrPages& other)
{
  (void)other;
}

ErrPages& ErrPages::operator=(const ErrPages& other)
{
  (void)other;
  return (*this);
}

ErrPages::~ErrPages()
{}

// ------------------------------=[ Methods ]=------------------------------ //
//

str ErrPages::_getTemplateWithErrStr(const str& s)
{
  str ret =
      "<html>\n"
      "<head>\n"
      "<title>m0fr1m's Webserv Error Page</title>\n"
      "<style>\n"
      "body {\n"
      "margin: 300px 100px 0px 100px;\n"
      "text-align: center;\n"
      "}\n"
      ".msg {\n"
      "margin: 200px;\n"
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
      "</h1>\n"
      "</div>\n"
      "</body>\n"
      "</html>\n";
  return ret;
}

str ErrPages::getDefaultErrPage(u16 code)
{
  str errstr;
  switch (code) {
  case 403:
    errstr = "403 - Forbidden";
    break;
  case 404:
    errstr = "404 - Page Not Found";
    break;
  case 405:
    errstr = "404 - Method Not Allowed";
    break;
  case 413:
    errstr = "413 - Content Too Large";
    break;
  case 418:
    errstr = "418 - I'm a teapot";
    break;
  case 500:
    errstr = "500 - Internal Server Error";
    break;
  case 501:
    errstr = "501 - Not Implemented";
    break;
  case 502:
    errstr = "502 - Bad Gateway";
    break;
  default:
    errstr += int2str(code) + " - Unknown HTML Status Code!";
  }
  return _getTemplateWithErrStr(errstr);
}

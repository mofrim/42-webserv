/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 19:12:58 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/11 17:42:11 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Request.hpp"

#include <stdexcept>
#include <unistd.h>

class VServer;
class Client;

class RequestHandler {
  private:
    Client *_cli;
    str     _vsrvName;

    char _buffer[READ_BUFSIZE];

    // unused
    RequestHandler(const RequestHandler& o);
    RequestHandler& operator=(const RequestHandler& o);

    void _setVirtualServerFromHeader();

    str _getErrPage(e_HTTPStatus c);

  public:
    RequestHandler();
    RequestHandler(Client *cli);
    ~RequestHandler();

    void readRequest();
    void readRequestServerless();
    void writeResponse();

    void setVsrvName(const str& n);

    class ReqHandlerException: public std::runtime_error {
      public:
        ReqHandlerException(const std::string& msg);
    };
};

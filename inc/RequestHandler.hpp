/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 19:12:58 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/19 08:39:52 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Request.hpp"

#include <stdexcept>

class VServer;
class Client;

class RequestHandler {
  private:
    Client *_cli;
    str     _vsrvName;

    char _buffer[READ_BUFSIZE];

    size_t _respoBytesSent;
    size_t _respoSize;

    // unused
    RequestHandler(const RequestHandler& o);
    RequestHandler& operator=(const RequestHandler& o);

    void _setVirtualServerFromHeader();

  public:
    RequestHandler();
    RequestHandler(Client *cli);
    ~RequestHandler();

    void readRequest();
    void readRequestServerless();
    void writeResponse();

    void setVsrvName(const str& n);

    void reset();

    class ReqHandlerException: public std::runtime_error {
      public:
        ReqHandlerException(const std::string& msg);
    };
};

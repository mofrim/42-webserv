/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 19:12:58 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/26 16:36:43 by fmaurer          ###   ########.fr       */
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

  public:
    RequestHandler();
    RequestHandler(Client *cli);
    ~RequestHandler();

    void readRequest();
    void writeResponse();

    void setVsrvname(const str& n);

    class ReqHandlerException: public std::runtime_error {
      public:
        ReqHandlerException(const std::string& msg);
    };
};

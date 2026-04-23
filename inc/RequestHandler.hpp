/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 19:12:58 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/22 15:03:47 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include "Request.hpp"

#include <deque>
#include <map>
#include <stdexcept>
#include <unistd.h>

// FIXME: for now this is the read buffer size. definitely coud be higher.
#define READ_BUFSIZE 12

class VServer;

class RequestHandler {
  private:
    VServer *_srv;

    // choosing a deque here as we are going to insert new request at the
    // beginning
    std::deque<Request> _requests;

    std::map< Client *, std::deque<Request> > _reqQueue;

    // FIXME:
    // maybe move cli param to a seperate private field so i don't have to pass
    // it to every function
    bool     _cliHasUnfinishedRequest(Client *cli);
    Request& _getUnfinishedReq(Client *cli);

    RequestHandler(const RequestHandler& other);
    RequestHandler();

  public:
    RequestHandler(VServer *srv);
    RequestHandler& operator=(const RequestHandler& other);
    ~RequestHandler();

    int readRequest(Client *cli);
    int writeResponse(Client *cli);

    class ReqHandlerException: public std::runtime_error {
      public:
        ReqHandlerException(const std::string& msg);
    };
};

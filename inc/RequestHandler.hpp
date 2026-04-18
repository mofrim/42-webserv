/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 19:12:58 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/18 16:00:14 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "Request.hpp"

#include <deque>
#include <stdexcept>
#include <unistd.h>

// FIXME: for now this is the read buffer size. definitely coud be higher.
#define READ_BUFSIZE 4096

class VServer;

class RequestHandler {
  private:
    VServer *_srv;

    // choosing a deque here as we are going insert new request at the beginning
    std::deque<Request> _requests;

    RequestHandler(const RequestHandler& other);
    RequestHandler();

  public:
    RequestHandler(VServer *srv);
    RequestHandler& operator=(const RequestHandler& other);
    ~RequestHandler();

    int readRequest(int fd);
    int writeResponse(int fd);

    class ReqHandlerException: public std::runtime_error {
      public:
        ReqHandlerException(const std::string& msg);
    };
};
#endif

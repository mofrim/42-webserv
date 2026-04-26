/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 19:11:06 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/25 22:04:06 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "WsrvLib.hpp"

#include <map>

class Request;
class Client;
class VServer;

class Response {
  private:
    u16                _statusCode;
    t_RequestLine      _reqline;
    std::map<str, str> _reqHeaders;
    Client            *_cli;
    VServer           *_vsrv;
    std::map<str, str> _respoHeaders;
    str                _body;
    str                _mimeType;
    str                _respoStr;

    void _setFieldsFromReq(const Request& req);
    void _getBody();
    void _buildRespoHdrs();

  public:
    Response();
    Response(const Response& other);
    Response& operator=(const Response& other);
    Response(const Request& req);
    ~Response();

    u16 genResponse(const Request& req);
    str getStr() const;

    void reset();
};

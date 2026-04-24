/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 19:11:06 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/24 17:31:35 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include "WsrvLib.hpp"

#include <map>

class Request;

class Response {
  private:
    u16                _statusCode;
    t_RequestLine      _reqline;
    std::map<str, str> _reqHeaders;

    Client  *_cli;
    VServer *_vsrv;

    std::map<str, str> _respoHeaders;

    // TODO: everything about body handling
    str _body;

    str _mimeType;

    void _setFieldsFromReq(const Request& req);
    void _getBody();
    void _buildRespoHdrs();
    str  _getMimeType(const str& p);

    str _respoStr;

  public:
    Response();
    Response(const Response& other);
    Response& operator=(const Response& other);
    Response(const Request& req);
    ~Response();

    void genResponse(const Request& req);
    str  getStr() const;
};

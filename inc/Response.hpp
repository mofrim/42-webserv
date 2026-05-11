/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 19:11:06 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/11 15:10:34 by fmaurer          ###   ########.fr       */
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
    // FIXME: Set them as pointers from Request for convenient access in here.
    // Or do something more elegant than just copying them over.
    e_HTTPStatus       _statusCode;
    t_RequestLine      _reqline;
    std::map<str, str> _reqHeaders;
    Client            *_cli;
    VServer           *_vsrv;
    bool               _closeConn;

    std::map<str, str> _respoHeaders;
    str                _body;
    str                _mimeType;
    str                _respoStr;

    void _setFieldsFromReq(const Request& req);
    void _getBody();
    void _buildRespoHdrs();
    void _genResponse();

    static std::map<str, str> _buildErrRespoHdrs(
        e_HTTPStatus status, const str& body);

  public:
    Response();
    Response(const Response& other);
    Response& operator=(const Response& other);
    ~Response();

    e_HTTPStatus genResponse(const Request& req);
    str          getRespoStr() const;

    void reset();

    static str genErrResponse(e_HTTPStatus errCode, constr errPage = "");
};

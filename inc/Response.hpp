/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 19:11:06 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/13 16:20:22 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Route.hpp"
#include "WsrvLib.hpp"

#include <map>

class Request;
class Client;
class VServer;

class Response {
  private:
    Response(const Response& other);

    // FIXME: Set them as pointers from Request for convenient access in here.
    // Or do something more elegant than just copying them over.
    e_HTTPStatus  _status;
    t_RequestLine _reqline;
    Client       *_cli;
    VServer      *_vsrv;
    bool          _closeConn;

    std::map<str, str> _respoHeaders;
    str                _body;
    str                _mimeType;
    str                _respoStr;

    Route *_matchedRoute;
    str    _targetPath; // the target-path minus the route

    Request *_req;

    void _setFieldsFromReq(Request& req);
    void _getBody200();
    void _getBody400();
    void _readBodyFromFile(constr& path);
    void _buildRespoHdrs();
    void _genResponse();

    void _handleBadRequest();
    void _handleRedir();
    void _handleCGI();
    void _handleSimplePost();
    void _handleDelete();

    static std::map<str, str> _buildErrRespoHdrs(
        e_HTTPStatus status, const str& body);

  public:
    Response();
    Response& operator=(const Response& other);
    ~Response();

    e_HTTPStatus generateResponse(Request& req);
    str          getRespoStr() const;

    void reset();

    static str genDefaultErrResponse(e_HTTPStatus errCode, constr errPage = "");
};

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 19:11:06 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/17 18:05:03 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Route.hpp"
#include "WsrvLib.hpp"

#include <map>
#include <sys/wait.h>

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
    str                _vsrvName;
    e_Method           _method;

    Route *_matchedRoute;
    str    _targetPath; // the target-path minus the route

    Request *_req;

    void _setFieldsFromReq(Request& req);
    void _getBody200();
    void _getBody400();
    void _readBodyFromFile(constr& path, bool setErrPageOnFail = true);
    void _buildRespoHdrs();
    void _buildResponseStr();
    void _setBodyStatusPage(constr& opts = "");

    void _handleBadRequest();
    void _handleSimplePost();
    void _handleSimplePostFile(
        constr& upDir, constr& mimeType = "application/octet-stream");
    void _handleSimplePostForm(constr& upDir, constr& mimeType);
    void _handleDelete();
    void _handleRedir();

    // -------------------------------=[ CGI ]=------------------------------ //

    int   _cgiParentWriteFd;
    int   _cgiParentReadFd;
    pid_t _cgiPid;
    str   _cgiBody;

    void                _cgiHandleBadScript(constr& s);
    std::map<str, str>  _cgiEvalScriptPath();
    std::pair<str, int> _cgiDetermineScriptFile();
    char              **_cgiBuildEnv(std::map<str, str> cgiParams);
    e_HTTPStatus        _cgiSetup(std::map<str, str> cgiParams);

    static std::map<str, str> _buildErrRespoHdrs(
        e_HTTPStatus status, const str& body);

  public:
    Response();
    Response& operator=(const Response& other);
    ~Response();

    e_HTTPStatus generateResponse(Request& req);
    str          getRespoStr() const;
    e_HTTPStatus handleCGI(Request& req);

    void cgiWrite();
    bool cgiWait();
    void cgiRead();
    void cgiProcessBody();
    void cgiShutdown();

    void reset();

    e_HTTPStatus getStatus() const;

    static str genDefaultErrResponse(e_HTTPStatus errCode, constr errPage = "");
};

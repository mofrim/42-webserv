/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 19:11:06 by fmaurer           #+#    #+#             */
/*   Updated: 2026/06/24 15:08:42 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Route.hpp"
#include "WsrvLib.hpp"

#include <map>
#include <sys/wait.h>

// 2^14 bytes should be enough, no?
#define CGI_READBUFSIZE 16384

class Request;
class Client;
class VServer;

// we want to be positive (to make checking for errors via > 0 testing possible)
// but not interfer with any exit status code, so 4242 is the natural choice.
#define CHILD_GONE 4242
#define CHILD_RUNNING -1

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
    bool _respoHdrHas(constr& hdr);
    void _setBodyStatusPage(constr& opts = "");

    static std::map<str, str> _buildErrRespoHdrs(
        e_HTTPStatus status, const str& body);

    void _handleSimplePost();
    void _handleSimplePostFile(
        constr& upDir, constr& mimeType = "application/octet-stream");
    void _handleSimplePostForm(constr& upDir, constr& mimeType);
    void _handleDelete();
    void _handleRedir();

    // -------------------------------=[ CGI ]=------------------------------ //

    int    _cgiParentWriteFd;
    int    _cgiParentReadFd;
    pid_t  _cgiPid;
    str    _cgiBody;
    size_t _cgiBytesWritten;
    size_t _cgiWriteBodySize;
    char   _cgiReadBuffer[CGI_READBUFSIZE];

    void                _cgiHandleBadScript(constr& s);
    std::map<str, str>  _cgiEvalScriptPath();
    std::pair<str, int> _cgiDetermineScriptFile();
    void                _cgiBuildEnvPathsAndURI(
        std::map<str, str>& cgiParams, constr& cgiScriptRelativePath);
    char       **_cgiBuildEnv(std::map<str, str>& cgiParams);
    e_HTTPStatus _cgiSetup(std::map<str, str>& cgiParams);

  public:
    Response();
    Response& operator=(const Response& other);
    ~Response();

    e_HTTPStatus buildResponse(Request& req);
    const str&   getRespoStr() const;

    e_HTTPStatus handleCGI(Request& req);
    void         cgiWrite();
    int          cgiEvalChildState();
    void         cgiRead();
    void         cgiProcessBody();
    void         cgiCleanupFds();
    bool         cgiIsWriteFd(int fd) const;
    bool         cgiIsRead(int fd) const;
    pid_t        cgiGetCpid() const;

    void reset();

    e_HTTPStatus getStatus() const;
    void         setStatus(e_HTTPStatus s);

    static str genDefaultErrResponse(e_HTTPStatus errCode, constr errPage = "");
};

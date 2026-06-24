/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 08:52:11 by fmaurer           #+#    #+#             */
/*   Updated: 2026/06/24 18:04:40 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "VServerCfg.hpp"

#include <fstream>
#include <set>
#include <stack>
#include <vector>

#define BANG '!'

class ConfigParser {
  private:
    // don't need them
    ConfigParser();
    ConfigParser(const ConfigParser& other);
    ConfigParser& operator=(const ConfigParser& other);

    std::ifstream           _cfgfile;
    const str               _cfgFname;
    std::vector<VServerCfg> _vcfgs;
    bool                    _bad;
    Route                   _currentRoute;

    // -------------------=[   Tokenizer related things ]=------------------- //

    static const std::set<str> _knownDirectives;
    static std::set<str>       _initKnownDirectives();

    enum e_CfgScope { S_GLOBAL, S_SERVER, S_ROUTE };
    std::stack<e_CfgScope> _scope;

    enum e_TokType {
      TOK_DIREC,  // listen, route
      TOK_BANG,   // !
      TOK_BSTART, // {
      TOK_BEND,   // }
      TOK_NAME,   // 42.de
      TOK_IFACE,  // localhost:1234
      TOK_BYTES,  // 10000000
      TOK_FSPATH, // ./www
      TOK_PATH,   // /upload
      TOK_ROUTE,  // /moep/miep -> no dots!
      TOK_ERROR,  // 404:/moep/miep/404.html -> route + filename but has to be
                  // under root
      TOK_FNAME,  // index.html -> to specify multiple: new line, new direc
      TOK_BOOL,   // true, false
      TOK_METH,   // GET POST ...
      TOK_CGI,    // py:/usr/bin/env python3
      TOK_REDIR,  // /moep or https://42.fr
      TOK_TIME,   // seconds. 3600 = 1h is max!
      TOK_NULL
    };

    enum e_Direcs {
      DIR_SERVER,
      DIR_SERVERNAME,
      DIR_ROUTE,
      DIR_LISTEN,
      DIR_MAXBODYSIZE,
      DIR_ERRORPAGE,
      DIR_INDEX,
      DIR_AUTOINDEX,
      DIR_METHODS,
      DIR_ROOT,
      DIR_UPLOAD,
      DIR_REDIRECT,
      DIR_CGI,
      DIR_TIMEREQ,
      DIR_TIMECGI,
      DIR_INVALID
    };

    // for TOK_DIREC val will be unused. Did this becaus using a union here
    // needs sooo much boilerplate.
    typedef struct {
        e_TokType type;
        str       val;
        e_Direcs  direc;
        size_t    line;
    } t_Token;

    // have to write this way to often -> shorten it

    std::vector<t_Token>           _tokens;
    std::vector<t_Token>::iterator _tokIt;

    e_TokType _nextTokFromDirec(e_Direcs direc);
    e_Direcs  _str2direc(const str& s);
    void _readTokVal(t_Token& tok, str::iterator& it, const str::iterator& end);
    t_Token _readTok(
        str::iterator& it, const str::iterator& end, const t_Token& lasttok);
    str  _direc2str(e_Direcs d) const;
    str  _toktype2str(e_TokType t) const;
    void _evalScope(const t_Token& tok);
    void _advanceTillSrvEnd();
    void _tokenize();

    // ------------------------=[ The REAL Parsing ]=------------------------ //

    void _processTokens();
    bool _parseVServer(VServerCfg& vcfg);
    void _skipFooTokens();
    bool _isMetaToken() const;
    bool _parseServerDirec(VServerCfg& vcfg);
    bool _parseRouteDirec(VServerCfg& vcfg);

    // ----------------------=[ Global Scope Parsing ]=---------------------- //

    bool _parseTokTime();

    // ----------------------=[ Server Scope Parsing ]=---------------------- //

    bool _parseTokName(VServerCfg& vcfg);  // serverName
    bool _parseTokRoute();                 // route
    bool _parseTokIface(VServerCfg& vcfg); // listen
    bool _parseTokError(VServerCfg& vcfg); // errorPage

    // ----------------------=[ Route Scope Parsing ]=----------------------- //

    bool _parseTokFname(); // index
    bool _parseTokBool();  // autoindex
    bool _parseTokMeth();  // methods
    bool _parseTokRedir(); // redirect
    bool _parseTokCgi();   // cgi
    bool _parseTokPath();  // upload

    // ----------------------=[ Mixed Scope Parsing ]=----------------------- //

    bool _parseTokBytes(VServerCfg& vcfg);  // maxBodySize
    bool _parseTokFspath(VServerCfg& vcfg); // root

  public:
    ConfigParser(const str& cfgFname);
    ~ConfigParser();

    void parse();

    bool                           bad() const;
    const std::vector<VServerCfg>& getCfgs() const;

    static std::set<str> getKnownDirectives();

    void dbgPrintTokens() const;
};

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 08:52:11 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/08 11:38:21 by fmaurer          ###   ########.fr       */
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
      TOK_STATUS, // 404, 400, ... -> TOK_FSPATH
      TOK_FSPATH, // ./www
      TOK_ROUTE,  // /moep/miep -> no dots!
      TOK_FROUTE, // /moep/miep/404.html -> route + filename but has to be
                  // under root
      TOK_FNAME,  // index.html -> to specify multiple: new line, new direc
      TOK_BOOL,   // true, false
      TOK_METH,   // GET POST ...
      TOK_CGI,    // py:/usr/bin/env python3
      TOK_REDIR,  // /moep or https://42.fr
      TOK_NULL
    };

    enum e_Direcs {
      DIR_SERVER,
      DIR_SERVERNAME,
      DIR_ROUTE,
      DIR_LISTEN,
      DIR_MAXBODYSIZE,
      DIR_ERRORPAGE,
      DIR_DEFAULTFILE,
      DIR_AUTOINDEX,
      DIR_METHODS,
      DIR_ROOT,
      DIR_UPLOAD,
      DIR_REDIRECT,
      DIR_INDEX,
      DIR_CGI,
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

    std::vector<t_Token> _tokens;

    e_TokType _nextTokFromDirec(e_Direcs direc);
    e_Direcs  _str2direc(const str& s);
    void _readTokVal(t_Token& tok, str::iterator& it, const str::iterator& end);
    t_Token _readTok(
        str::iterator& it, const str::iterator& end, const t_Token& lasttok);
    void _tokenize();

  public:
    ConfigParser(const str& cfgFname);
    ~ConfigParser();

    void                           parse();
    bool                           bad() const;
    const std::vector<VServerCfg>& getCfgs() const;

    static std::set<str> getKnownDirectives();
};

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Route.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 16:41:56 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/19 13:08:25 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "WsrvLib.hpp"

// A Route must be able to store data for a config like this

// TODO: add everything i want to parse!
class Route {
  private:
    str  _path;
    bool _autoindex;
    str  _index;
    str  _root;
    u32  _maxBodySize;
    str  _upload;

    std::set<e_Method> _methods;

    std::map<e_HTTPStatus, str> _errPages;

    std::pair<e_HTTPStatus, URI> _redir;

    // Ex.: key = "py", val = "/usr/bin/env python"
    std::map<str, str> _cgi;

  public:
    Route();
    Route(const Route& other);
    Route& operator=(const Route& other);
    ~Route();

    void setPath(str p);
    str  getPath() const;

    void setAutoindex(bool a);
    bool isAutoindex() const;

    void setIndex(const str& s);
    str  getIndex() const;

    void setRoot(str root);
    str  getRoot() const;

    void                        addErrPage(e_HTTPStatus s, const str& path);
    str                         getErrPage(e_HTTPStatus s);
    std::map<e_HTTPStatus, str> getErrPages() const;

    void setMaxBodySize(u32 mbs);
    u32  getMaxBodySize() const;

    const std::set<e_Method>& getMethods() const;

    // std::set::insert returns a pair of which the second item is a boolean
    // which indicates if the item already existed. this we return here.
    bool addMethod(e_Method m);
    void clearMethods();

    const std::pair<e_HTTPStatus, URI>& getRedir() const;
    void setRedir(e_HTTPStatus s, const str& url);

    const std::map<str, str>& getCgi() const;
    void                      addCgi(constr& ext, constr& exec);

    void setUpload(str p);
    str  getUpload() const;

    void reset();
};

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Route.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 16:41:56 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/09 15:24:34 by fmaurer          ###   ########.fr       */
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
    str  _default_file;
    str  _root;
    u32  _maxBodySize;

    std::map<e_HTTPStatus, str> _errPages;

  public:
    Route();
    Route(const Route& other);
    Route& operator=(const Route& other);
    ~Route();

    void setPath(str p);
    str  getPath() const;

    void setAutoindex(bool a);
    bool getAutoindex() const;

    void setDefaultFile(const str& s);
    str  getDefaultFile() const;

    void setRoot(str root);
    str  getRoot() const;

    void                        addErrPage(e_HTTPStatus s, const str& path);
    str                         getErrPage(e_HTTPStatus s);
    std::map<e_HTTPStatus, str> getErrPages() const;

    void setMaxBodySize(u32 mbs);
    u32  getMaxBodySize() const;

    void reset();
};

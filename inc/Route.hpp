/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Route.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 16:41:56 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/24 17:32:19 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "WsrvLib.hpp"

// A Route must be able to store data for a config like this

class Route {
  private:
    str  _path;
    bool _autoindex;
    str  _default_file;
    str  _root;

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
};

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   URL.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/06 22:32:20 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/07 10:23:12 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WsrvLib.hpp"

#include <map>

// URL class
//
// There will be 2 canonical ways of instantiating an URL:
//
//  1) via defaullt ctor and URL::parseURL
//  2) via URL(const str& target) ctor
//
// In both cases URL::bad() should be checked.
class URL {
  private:
    str                _path;
    std::map<str, str> _query;
    str                _fragment;

    bool _bad;
    bool _empty;

  public:
    URL();
    URL(const URL& other);
    URL& operator=(const URL& other);
    ~URL();
    URL(const str& u);

    str parseTargetURL(const str& t);

    bool bad() const;
    bool empty() const;

    str                getPath() const;
    std::map<str, str> getQuery() const;
};

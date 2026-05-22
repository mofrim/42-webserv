/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBody.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 10:17:58 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/22 12:46:36 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// this is a simple wrapper class for handling request bodies. by using a class
// we can easily handle a possible file buffering later  by hiding it behind
// this class interface.

#pragma once

#include "WsrvLib.hpp"

class RequestBody {
  private:
    char  *_bodyData;
    size_t _size;
    size_t _maxSize;

  public:
    RequestBody();
    RequestBody(const RequestBody& other);
    RequestBody& operator=(const RequestBody& other);
    ~RequestBody();

    const char *getBodyData() const;
    int         appendData(const char *dat, size_t len);
    void        reset();
    bool        setMaxSize(size_t mbs);
    size_t      getSize() const;
};

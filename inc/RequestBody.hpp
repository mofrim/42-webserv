/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBody.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 10:17:58 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/30 08:09:06 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// this is a simple wrapper class for handling request bodies. by using a class
// we can easily handle a possible file buffering later by hiding it behind
// this class interface.

#pragma once

#include "WsrvLib.hpp"

// we allow 100 bytes for the chunk header. this should be more than enough
#define MAX_CHUNK_HEADER_SIZE 100

class RequestBody {
  private:
    char  *_bodyData;
    size_t _size;
    size_t _maxSize;

    bool   _isChunked;
    u32    _chunkSize;
    size_t _bytesReadChunk;
    str    _chunkBuffer;
    bool   _lastChunkRead;
    bool   _chunkedComplete;

    int _appendChunked();
    int _parseChunkSize();

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
    void        setChunked();
    bool        isChunkedComplete() const;
};

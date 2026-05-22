/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 10:18:32 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/22 12:56:16 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "RequestBody.hpp"
#include "utils.hpp"

#include <cstring>

// --------------------------------=[ OCF ]=-------------------------------- //

RequestBody::RequestBody(): _bodyData(NULL), _size(0), _maxSize(0) {}

RequestBody::RequestBody(const RequestBody& o)
{
  if (this != &o)
    *this = o;
}

RequestBody& RequestBody::operator=(const RequestBody& o)
{
  if (this != &o) {
    _size    = o._size;
    _maxSize = o._maxSize;

    if (_bodyData != NULL)
      delete[] _bodyData;
    _bodyData = NULL;

    if (o._bodyData != NULL) {
      _bodyData = new char[o._maxSize];
      if (_bodyData == NULL)
        throw std::runtime_error(
            "(RequestBody::operator=) Allocation for _bodyData failed!");
      memcpy(_bodyData, o._bodyData, o._size);
    }
  }
  return (*this);
}

RequestBody::~RequestBody()
{
  if (_bodyData != NULL)
    delete[] _bodyData;
}

// -------------------------------=[ other ]=------------------------------- //

const char *RequestBody::getBodyData() const { return _bodyData; }

// append raw bytes to bodyData
// returns 1 on success, 0 if data was truncated, -1 if insert to _bodyData
// failed.
int RequestBody::appendData(const char *dat, size_t len)
{
  int ret = 1;

  // truncate data if _maxSize would be exceeded
  if (len + _size > _maxSize) {
    len = _maxSize - _size;
    ret = 0;
  }

  try {
    memcpy(_bodyData + _size, dat, len);
    _size += len;
  } catch (const std::exception& e) {
    ret = -1;
  }
  Logger::logBug(
      "bodies CONTAINER REAL size after appending: " + int2str(_size));
  return ret;
}

// set _bodyData's capacity to mbs only have one big allocation here. we'll
// see how that goes...
bool RequestBody::setMaxSize(size_t mbs)
{
  _bodyData = new char[mbs];
  if (_bodyData == NULL)
    throw std::runtime_error(
        "(RequestBody::setMaxBodySize) Allocating space for body data failed");
  _maxSize = mbs;
  return OK;
}

// cleanup
void RequestBody::reset()
{
  if (_bodyData != NULL)
    delete[] _bodyData;
  _bodyData = NULL;
  _size     = 0;
  _maxSize  = 0;
}

size_t RequestBody::getSize() const { return _size; }

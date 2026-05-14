/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 10:18:32 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/14 19:43:00 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "RequestBody.hpp"

// --------------------------------=[ OCF ]=-------------------------------- //

RequestBody::RequestBody(): _size(0) {}

RequestBody::RequestBody(const RequestBody& o)
{
  if (this != &o) {
    _bodyData = o._bodyData;
    _size     = o._size;
  }
}

RequestBody& RequestBody::operator=(const RequestBody& o)
{
  if (this != &o) {
    _bodyData = o._bodyData;
    _size     = o._size;
  }
  return (*this);
}

RequestBody::~RequestBody() {}

// -------------------------------=[ other ]=------------------------------- //

// assign raw bytes to bodyData
// FIXME ooh, these exceptions. how could i really handle them?!
void RequestBody::setBodyData(const char *data, size_t len)
{
  try {
    _bodyData.assign(data, data + len);
    _size = len;
  } catch (const std::exception& e) {
    Logger::log_err("RequestBody::setBodyData", "Could assign to _bodyData");
  }
}

str RequestBody::getBodyDataAsStr()
{
  str body;
  try {
    body.assign(_bodyData.begin(), _bodyData.end());
  } catch (const std::exception& e) {
    Logger::log_err("RequestBody::getBodyDataAsStr", "Failed to assign string");
  }
  return body;
}

std::vector<char>& RequestBody::getBodyData() { return _bodyData; }

// append raw bytes to bodyData
// FIXME ooh, these exceptions. how could i really handle them?!
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
    _bodyData.insert(_bodyData.end(), dat, dat + len);
    _size += len;
  } catch (const std::exception& e) {
    ret = -1;
  }
  return ret;
}

// set _bodyData's capacity to mbs only have one big allocation here. we'll see
// how that goes...
bool RequestBody::setMaxSize(size_t mbs)
{
  try {
    _bodyData.resize(mbs);
    _maxSize = mbs;
  } catch (const std::exception& e) {
    Logger::log_err("RequestBody::setMaxBodySize", "Resizing _bodyData failed");
    return KO;
  }
  return OK;
}

// cleanup
void RequestBody::reset()
{
  _bodyData.clear();
  _size = 0;
}

size_t RequestBody::getSize() { return _size; }

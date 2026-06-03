/* ************************************************************************** */ /*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 10:18:32 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/28 07:52:45 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "RequestBody.hpp"
#include "utils.hpp"

#include <cstring>

// --------------------------------=[ OCF ]=-------------------------------- //

RequestBody::RequestBody():
  _bodyData(NULL), _size(0), _maxSize(0), _isChunked(false), _chunkSize(0),
  _bytesReadChunk(0), _lastChunkRead(false), _chunkedComplete(false)
{}

RequestBody::RequestBody(const RequestBody& o)
{
  if (this != &o)
    *this = o;
}

RequestBody& RequestBody::operator=(const RequestBody& o)
{
  if (this != &o) {
    _size      = o._size;
    _maxSize   = o._maxSize;
    _isChunked = o._isChunked;

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
// failed, -2 if chunked request got too big
int RequestBody::appendData(const char *dat, size_t len)
{
  if (dat == NULL)
    return -1;

  if (len == 0)
    return 1;

  int ret = 1;

  // tuncate only if not chunked. if chunked, handle a possibly too large body
  // in _appendChunked
  if (!_isChunked && len + _size > _maxSize) {
    len = _maxSize - _size;
    ret = 0;
  }

  if (_isChunked) {
    _chunkBuffer.append(dat, len);
    int appRet = _appendChunked();
    ret        = (appRet < 0 ? appRet : ret);
  }
  else {
    memcpy(_bodyData + _size, dat, len);
    _size += len;
  }

  Logger::logBug("ret: " + int2str(ret));

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
  _bodyData        = NULL;
  _size            = 0;
  _maxSize         = 0;
  _isChunked       = false;
  _chunkSize       = 0;
  _bytesReadChunk  = 0;
  _lastChunkRead   = false;
  _chunkedComplete = false;
  _chunkBuffer.clear();
}

size_t RequestBody::getSize() const { return _size; }

void RequestBody::setChunked() { _isChunked = true; }

bool RequestBody::isChunkedComplete() const { return _chunkedComplete; }

// returns...
//
// - 0 if a chunk size could be parsed
// - 1 if no chunk-size was found yet
// - -1 if anything went wrong
int RequestBody::_parseChunkSize()
{
  size_t crlfPos = findCRLF(_chunkBuffer.data(), _chunkBuffer.size());

  if (_chunkBuffer.size() > MAX_CHUNK_HEADER_SIZE && crlfPos == str::npos) {
    Logger::logWarn(
        "RequestBody::_appendChunked", "No chunk size found -> 400");
    return -1;
  }

  if (crlfPos == 0) {
    Logger::logWarn(
        "RequestBody::_appendChunked", "Chunk started with CRLF -> 400");
    return -1;
  }

  if (crlfPos == str::npos)
    return 1;

  str csStr = _chunkBuffer.substr(0, crlfPos);

  // split off any chunk-ext bla blub
  if (csStr.find(';') != str::npos)
    csStr = csStr.substr(0, csStr.find(';'));

  if (parseU32HexStr(csStr, _chunkSize) == false) {
    Logger::logWarn("RequestBody::_appendChunked", "Invalid chunk size!");
    return -1;
  }

  if (_chunkSize != 0)
    _chunkBuffer = _chunkBuffer.substr(crlfPos + 2);
  else
    //  leave the CRLF if we have read the last chunk size bc we want to detect
    //  the request end by CRLFX2
    _chunkBuffer = _chunkBuffer.substr(crlfPos);

  return 0;
}

// check if there is a complete chunk in the chunkBuffer. if so, append it to
// the req-body, remove it from chunkBuffer and reset _chunkSize to 0.
// returns:
//
//  * 0 if a chunk was appended
//  * 1 if no chunk could be appended
//  * -1 if the chunkBuffer size is bigger then the lastChunk sequence but the
//       sequence is not found!
int RequestBody::_appendChunkToBody()
{
  Logger::logBug("_chunkBuffer.size(): " + int2str(_chunkBuffer.size()));
  if (_chunkBuffer.size() >= _chunkSize + 2 &&
      _chunkBuffer.compare(_chunkSize, 2, CRLF) == 0)
  {
    // Logger::logBug("if-while - appending to chunkedBody: " +
    //     _chunkBuffer.substr(0, _chunkSize));
    memcpy(_bodyData + _size, _chunkBuffer.data(), _chunkSize);
    _size += _chunkSize;
    Logger::logBug(
        "if - body by now:\n" + printDataTrunc(_bodyData, _size, 100));

    _chunkBuffer = _chunkBuffer.substr(_chunkSize + 2);
    _chunkSize   = 0;
    return 0;
  }
  else if (_chunkBuffer.size() >= _chunkSize + 2 &&
      _chunkBuffer.compare(_chunkSize, 2, CRLF) != 0)
  {
    Logger::logDbg0("RequestBody::_appendChunkToBody",
        "Chunk too big for given _chunkSize or malformed");
    return -1;
  }

  return 1;
}

int RequestBody::_appendChunked()
{
  int ret = 1;

  if (_lastChunkRead) {
    if (_chunkBuffer.rfind(CRLFX2) != str::npos)
      _chunkedComplete = true;
    return 1;
  }

  if (_chunkSize == 0) {

    while (_chunkSize == 0 && (ret = _parseChunkSize()) == 0) {
      Logger::logDbg2("RequestBody::_appendChunked",
          "if - Chunk size: " + int2str(_chunkSize));
      // Logger::logBug("if - Chunk buffer so far:\n" +
      //     data2hexStr(_chunkBuffer.data(), _chunkBuffer.size()));

      // FIXME we certainly have to limit the size of the
      // trailer-section here!
      if (ret == 0 && _chunkSize == 0) {
        _lastChunkRead = true;
        if (_chunkBuffer.find(CRLFX2) != str::npos)
          _chunkedComplete = true;
        return 1;
      }

      if (_chunkSize + _size > _maxSize) {
        Logger::logDbg0(
            "RequestBody::_appendChunked", "chunkSize + bodySize too big!");
        return -2;
      }

      if (_appendChunkToBody() == -1)
        return -1;
    }
  }
  else {
    Logger::logBug("else - Chunk buffer so far:\n" +
        printDataTrunc(_chunkBuffer.data(), _chunkBuffer.size(), 100));

    if (_appendChunkToBody() == -1)
      return -1;

    while (_chunkSize == 0 && (ret = _parseChunkSize()) == 0) {
      if (ret < 0)
        return -1;
      Logger::logBug("Chunk size: " + int2str(_chunkSize));

      // Logger::logBug("Chunk buffer so far:\n" +
      //     data2hexStr(_chunkBuffer.data(), _chunkBuffer.size()));

      // FIXME we certainly have to limit the size of the
      // trailer-section here!
      if (ret == 0 && _chunkSize == 0) {
        _lastChunkRead = true;
        if (_chunkBuffer.find(CRLFX2) != str::npos)
          _chunkedComplete = true;
        return 1;
      }

      if (_appendChunkToBody() == -1)
        return -1;
    }
  }

  return ret;
}

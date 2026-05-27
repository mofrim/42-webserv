/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 20:52:55 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/27 06:25:14 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "utils.hpp"

#include <ctime>
#include <iostream>
#include <unistd.h>

// ------------=[ OCF, even though we don't need any of these ]=------------ //

Logger::Logger() {}

Logger::Logger(const Logger& other) { (void)other; }

Logger& Logger::operator=(const Logger& other)
{
  (void)other;
  return (*this);
}

Logger::~Logger() {}

// --------------------------------=[ init ]=-------------------------------- //

bool Logger::_isatty = true;

// yeah, i know. `isatty` is not listed in the external functions list, but it
// is just nice to have this feature for logging to files :)
void Logger::init() { _isatty = isatty(STDOUT_FILENO); }

// return current time. optional param bool turns adding brackets to returned
// str on/off.
//
// NOTE this is a stripped down version for better readability during eval. in
// prod i would prefer something like `%FT%H:%M:%S%z` as date string
str Logger::getLogtime(bool brackets)
{
  char       timestamp[101];
  time_t     current_time;
  struct tm *timeinfo;

  time(&current_time);
  timeinfo = localtime(&current_time);
  if (brackets)
    strftime(timestamp, 1000, "[%H:%M:%S]", timeinfo);
  else
    strftime(timestamp, 1000, "%H:%M:%S", timeinfo);

  return (str(timestamp));
}

void Logger::logErr(const str& msg)
{
  str logtime = getLogtime();
  if (_isatty)
    std::cout << BRED << logtime << " " << msg << RST << std::endl;
  else
    std::cout << logtime << " " << msg << std::endl;
}

void Logger::logErr(const str& pre, const str& msg)
{
  str logtime = getLogtime();
  if (_isatty)
    std::cout << RED << logtime << BRED << " (" << pre << ") " << RST << RED
              << msg << RST << std::endl;
  else
    std::cout << logtime << " (" << pre << ") " << msg << std::endl;
}

void Logger::logMsg(const str& msg)
{
  str logtime = getLogtime();
  if (_isatty)
    std::cout << BCYA << logtime << " " << msg << RST << std::endl;
  else
    std::cout << logtime << " " << msg << std::endl;
}

void Logger::logCfg(const str& msg)
{
  if (_isatty)
    std::cout << CYA << msg << RST << std::endl;
  else
    std::cout << msg << std::endl;
}

void Logger::logWarn(const str& msg)
{
  str logtime = getLogtime();
  if (_isatty)
    std::cout << YLO << logtime << " " << msg << RST << std::endl;
  else
    std::cout << logtime << " " << msg << std::endl;
}

void Logger::logWarn(const str& pre, const str& msg)
{
  str logtime = getLogtime();
  if (_isatty)
    std::cout << YLO << logtime << BYLO << " (" << pre << ") " << RST << YLO
              << msg << RST << std::endl;
  else
    std::cout << logtime << " (" << pre << ") " << msg << std::endl;
}

void Logger::logCfgErr(const size_t line, const str& msg)
{
  Logger::logWarn("cfg line " + int2str(line), msg);
}

void Logger::logDbg0(const str& msg)
{
  if (LOGLEVEL >= LOG_INFO) {
    str logtime = getLogtime();
    if (_isatty)
      std::cout << GRY << logtime << " " << msg << RST << std::endl;
    else
      std::cout << logtime << " " << msg << std::endl;
  }
}

void Logger::logDbg1(const str& msg)
{
  if (LOGLEVEL >= LOG_DEBUG) {
    str logtime = getLogtime();
    if (_isatty)
      std::cout << PNK << logtime << " " << msg << RST << std::endl;
    else
      std::cout << logtime << " " << msg << std::endl;
  }
}

void Logger::logDbg1(const str& pre, const str& msg)
{
  if (LOGLEVEL >= LOG_DEBUG) {
    str logtime = getLogtime();
    if (_isatty)
      std::cout << PNK << logtime << BPNK << " (" << pre << ") " << RST << PNK
                << msg << RST << std::endl;
    else
      std::cout << logtime << " (" << pre << ") " << msg << std::endl;
  }
}

void Logger::logDbg2(const str& msg)
{
  if (LOGLEVEL >= LOG_BRUTAL) {
    str logtime = getLogtime();
    if (_isatty)
      std::cout << BWHT << logtime << " " << msg << RST << std::endl;
    else
      std::cout << logtime << " " << msg << std::endl;
  }
}

void Logger::logDbg2(const str& pre, const str& msg)
{
  if (LOGLEVEL >= LOG_BRUTAL) {
    str logtime = getLogtime();
    if (_isatty)
      std::cout << BWHT << logtime << " (" << pre << ") " << msg << RST
                << std::endl;
    else
      std::cout << logtime << " (" << pre << ") " << msg << std::endl;
  }
}

void Logger::logSrv(const str& srvName, const str& msg, e_LogType logtype)
{
  if (LOGLEVEL >= LOG_INFO) {
    str txtcolr = (logtype == INFO ? GRY : YLO);
    str logtime = getLogtime();
    if (_isatty)
      std::cout << GRY << logtime << GRN << " (" << srvName << ") " << txtcolr
                << msg << RST << std::endl;
    else
      std::cout << logtime << " (" << srvName << ") " << msg << std::endl;
  }
}

// for logging response and requests in BRUTAL loglevel
void Logger::logReqRes(const str& srvName, const str& resreq, const str& data)
{
  if (LOGLEVEL >= LOG_BRUTAL) {
    str logtime = getLogtime();
    if (_isatty)
      std::cout << GRY << logtime << GRN << " (" << srvName << ") " << BGRY
                << resreq << ":" << BGRY << "\n---" << std::endl;
    else
      std::cout << logtime << " (" << srvName << ") " << resreq << ":"
                << "\n---" << std::endl;
    if (resreq != "Processing Request")
      std::cout << data2hexStr(data.data(), data.size());
    else
      std::cout << data;
    if (_isatty)
      std::cout << "\n---" << RST << std::endl;
    else
      std::cout << "\n---" << std::endl;
  }
}

// use this for bug hunting
void Logger::logBug(const str& msg)
{
  if (_isatty)
    std::cout << BUG << "[BUG] " << msg << RST << std::endl;
  else
    std::cout << "[BUG] " << msg << std::endl;
}

// use this for bug hunting
void Logger::logBug(constr& pre, const str& msg)
{
  if (_isatty)
    std::cout << BUG << "[BUG] " << "(" << pre << ") " << msg << RST
              << std::endl;
  else
    std::cout << "[BUG] " << "(" << pre << ") " << msg << std::endl;
}

void Logger::drawCycleSep()
{
  if (LOGLEVEL >= LOG_DEBUG) {
    if (_isatty)
      std::cout
          << YLO
          << "\n  --------------------- epoll cycle ---------------------\n\n"
          << RST;
    else
      std::cout
          << "\n  --------------------- epoll cycle ---------------------\n\n";
  }
}

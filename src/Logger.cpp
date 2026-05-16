/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 20:52:55 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/16 12:39:07 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "utils.hpp"

#include <ctime>
#include <iostream>

Logger::Logger() {}

Logger::Logger(const Logger& other) { (void)other; }

Logger& Logger::operator=(const Logger& other)
{
  (void)other;
  return (*this);
}

Logger::~Logger() {}

// return current time. optional param bool turns adding brackets to returned
// str on/off.
str Logger::getLogtime(bool brackets)
{
  char       timestamp[101];
  time_t     current_time;
  struct tm *timeinfo;

  time(&current_time);
  timeinfo = localtime(&current_time);
  if (brackets)
    strftime(timestamp, 1000, "[%FT%H:%M:%S%z]", timeinfo);
  else
    strftime(timestamp, 1000, "%FT%H:%M:%S%z", timeinfo);

  return (str(timestamp));
}

void Logger::logErr(const str& msg)
{
  str logtime = getLogtime();
  std::cout << BRED << logtime << " " << msg << RST << std::endl;
}

void Logger::logErr(const str& pre, const str& msg)
{
  str logtime = getLogtime();
  std::cout << RED << logtime << BRED << " (" << pre << ") " << RST << RED
            << msg << RST << std::endl;
}

void Logger::logMsg(const str& msg)
{
  str logtime = getLogtime();
  std::cout << BCYA << logtime << " " << msg << RST << std::endl;
}

void Logger::logCfg(const str& msg)
{
  std::cout << CYA << msg << RST << std::endl;
}

void Logger::logWarn(const str& msg)
{
  str logtime = getLogtime();
  std::cout << YLO << logtime << " " << msg << RST << std::endl;
}

void Logger::logWarn(const str& pre, const str& msg)
{
  str logtime = getLogtime();
  std::cout << YLO << logtime << BYLO << " (" << pre << ") " << RST << YLO
            << msg << RST << std::endl;
}

void Logger::logCfgErr(const size_t line, const str& msg)
{
  Logger::logWarn("cfg line " + int2str(line), msg);
}

void Logger::logDbg0(const str& msg)
{
  if (LOGLEVEL >= LOG_INFO) {
    str logtime = getLogtime();
    std::cout << GRY << logtime << " " << msg << RST << std::endl;
  }
}

void Logger::logDbg1(const str& msg)
{
  if (LOGLEVEL >= LOG_DEBUG) {
    str logtime = getLogtime();
    std::cout << PNK << logtime << " " << msg << RST << std::endl;
  }
}

void Logger::logDbg1(const str& pre, const str& msg)
{
  if (LOGLEVEL >= LOG_DEBUG) {
    str logtime = getLogtime();
    std::cout << PNK << logtime << BPNK << " (" << pre << ") " << RST << PNK
              << msg << RST << std::endl;
  }
}

void Logger::logDbg2(const str& msg)
{
  if (LOGLEVEL >= LOG_BRUTAL) {
    str logtime = getLogtime();
    std::cout << BWHT << logtime << " " << msg << RST << std::endl;
  }
}

void Logger::logSrv(const str& srvName, const str& msg, e_LogType logtype)
{
  if (LOGLEVEL >= LOG_INFO) {
    str txtcolr = (logtype == INFO ? GRY : YLO);
    str logtime = getLogtime();
    std::cout << GRY << logtime << GRN << " (" << srvName << ") " << txtcolr
              << msg << RST << std::endl;
  }
}

// for logging response and requests
// is being from level LOG_DEBUG (== 1) on
//
// FIXME: refac to take Client as first param. and maybe even whole response bc
// we might want to print the headers seperate from the body
void Logger::logReqRes(const str& srvName, const str& resreq, const str& data)
{
  if (LOGLEVEL >= LOG_BRUTAL) {
    str logtime = getLogtime();
    std::cout << GRY << logtime << GRN << " (" << srvName << ") " << BGRY
              << resreq << ":" << BGRY << "\n---" << std::endl;
    std::cout << data;
    std::cout << "\n---" << RST << std::endl;
  }
}

// use this for bug hunting
void Logger::logBug(const str& msg)
{
  std::cout << BPUR << "[BUG] " << msg << RST << std::endl;
}

// use this for bug hunting
void Logger::logBug(constr& pre, const str& msg)
{
  std::cout << BPUR << "[BUG] " << "(" << pre << ") " << msg << RST
            << std::endl;
}

void Logger::drawCycleSep()
{
  std::cout << YLO
            << "\n  --------------------- epoll cycle ---------------------\n\n"
            << RST;
}

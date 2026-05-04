/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 20:52:55 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/03 23:31:53 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

#include <ctime>
#include <iostream>

Logger::Logger()
{}

Logger::Logger(const Logger& other)
{
  (void)other;
}

Logger& Logger::operator=(const Logger& other)
{
  (void)other;
  return (*this);
}

Logger::~Logger()
{}

str Logger::getLogtime()
{
  char       timestamp[101];
  time_t     current_time;
  struct tm *timeinfo;

  time(&current_time);
  timeinfo = localtime(&current_time);
  strftime(timestamp, 1000, "[%F %H:%M:%S] ", timeinfo);
  return (str(timestamp));
}

void Logger::log_err(const str& msg)
{
  str logtime = getLogtime();
  std::cout << BRED << logtime << msg << RST << std::endl;
}

void Logger::log_msg(const str& msg)
{
  str logtime = getLogtime();
  std::cout << BCYA << logtime << msg << RST << std::endl;
}

void Logger::log_warn(const str& msg)
{
  str logtime = getLogtime();
  std::cout << YLO << logtime << msg << RST << std::endl;
}

void Logger::log_dbg0(const str& msg)
{
  if (LOGLEVEL >= LOG_INFO) {
    str logtime = getLogtime();
    std::cout << GRY << logtime << msg << RST << std::endl;
  }
}

void Logger::log_dbg1(const str& msg)
{
  if (LOGLEVEL >= LOG_DEBUG) {
    str logtime = getLogtime();
    std::cout << PNK << logtime << msg << RST << std::endl;
  }
}

void Logger::log_dbg2(const str& msg)
{
  if (LOGLEVEL >= LOG_BRUTAL) {
    str logtime = getLogtime();
    std::cout << BWHT << logtime << msg << RST << std::endl;
  }
}

void Logger::log_srv(const str& srvName, const str& msg, e_LogType logtype)
{
  if (LOGLEVEL >= LOG_INFO) {
    str txtcolr = (logtype == INFO ? GRY : YLO);
    str logtime = getLogtime();
    std::cout << GRY << logtime << GRN << "(" << srvName << ") " << txtcolr
              << msg << RST << std::endl;
  }
}

// for logging response and requests
// is being from level LOG_DEBUG (== 1) on
//
// FIXME: refac to take Client as first param. and maybe even whole response bc
// we might want to print the headers seperate from the body
void Logger::log_reqres(const str& srvName, const str& resreq, const str& data)
{
  if (LOGLEVEL >= LOG_BRUTAL) {
    str logtime = getLogtime();
    std::cout << GRY << logtime << GRN << "(" << srvName << ") " << BGRY
              << resreq << ":" << BGRY << "\n---" << std::endl;
    std::cout << data;
    std::cout << "\n---" << RST << std::endl;
  }
}

// use this for bug hunting
void Logger::log_bug(const str& msg)
{
  std::cout << BPUR << "[BUG] " << msg << RST << std::endl;
}

void Logger::drawCycleSep()
{
  std::cout << YLO
            << "\n  --------------------- epoll cycle ---------------------\n\n"
            << RST;
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 20:52:55 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/26 20:23:52 by fmaurer          ###   ########.fr       */
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
  if (LOGLEVEL >= INFO) {
    str logtime = getLogtime();
    std::cout << GRY << logtime << msg << RST << std::endl;
  }
}

void Logger::log_dbg1(const str& msg)
{
  if (LOGLEVEL >= DEBUG) {
    str logtime = getLogtime();
    std::cout << WHT << logtime << msg << RST << std::endl;
  }
}

void Logger::log_dbg2(const str& msg)
{
  if (LOGLEVEL >= BRUTAL) {
    str logtime = getLogtime();
    std::cout << BWHT << logtime << msg << RST << std::endl;
  }
}

void Logger::log_srv(const str& srvName, const str& msg)
{
  if (LOGLEVEL >= INFO) {
    str logtime = getLogtime();
    std::cout << GRY << logtime << GRN << "(" << srvName << ") " << GRY << msg
              << RST << std::endl;
  }
}

// for logging response and requests
void Logger::log_reqres(const str& srvName, const str& resreq, const str& data)
{
  if (LOGLEVEL >= INFO) {
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

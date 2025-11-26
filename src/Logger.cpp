/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 20:52:55 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/25 10:25:02 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

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

std::string Logger::_get_logtime()
{
	char			 timestamp[101];
	time_t		 current_time;
	struct tm *timeinfo;

	time(&current_time);
	timeinfo = localtime(&current_time);
	strftime(timestamp, 1000, "[%F %H:%M:%S] ", timeinfo);
	return (std::string(timestamp));
}

void Logger::log_err(const std::string& msg)
{
	std::string logtime = _get_logtime();
	std::cout << RED << logtime << msg << RST << std::endl;
}

void Logger::log_msg(const std::string& msg)
{
	std::string logtime = _get_logtime();
	std::cout << CYA << logtime << msg << RST << std::endl;
}

void Logger::log_warn(const std::string& msg)
{
	std::string logtime = _get_logtime();
	std::cout << YLO << logtime << msg << RST << std::endl;
}

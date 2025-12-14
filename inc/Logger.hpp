/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 20:49:55 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/14 21:31:00 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

// shell colors
#define RED "\e[31m"
#define CYA "\e[36m"
#define YLO "\e[33m"
#define PUR "\e[34m"
#define RST "\e[0m"

enum { DEBUG, INFO };

#ifndef LOGLEVEL
#define LOGLEVEL INFO
#endif

// IDEA: try to pass a logfile to the Logger-Class via class global variable...
// is this even possible with this utility class?
class Logger {
	private:
		Logger();
		Logger(const Logger& other);
		Logger& operator=(const Logger& other);
		~Logger();

		static std::string _get_logtime();

	public:
		static void log_err(const std::string& msg);
		static void log_msg(const std::string& msg);
		static void log_warn(const std::string& msg);
		static void log_dbg(const std::string& msg);
};

#endif

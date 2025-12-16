/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 20:49:55 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/16 09:35:30 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

// shell colors
#define RED "\e[31m"
#define GRN "\e[32m"
#define YLO "\e[33m"
#define PUR "\e[34m"
#define PNK "\e[35m"
#define CYA "\e[36m"
#define GRY "\e[37m"
#define WHT "\e[38m"
#define RST "\e[0m"

// bold
#define BRED "\e[1;90m"
#define BGRN "\e[1;91m"
#define BYLO "\e[1;92m"
#define BPUR "\e[1;93m"
#define BPNK "\e[1;94m"
#define BCYA "\e[1;95m"
#define BGRY "\e[1;96m"
#define BWHT "\e[1;97m"
#define BRST "\e[1;98m"

enum { DEBUG = 0, INFO = 1 };

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
		static void log_dbg0(const std::string& msg);
		static void log_dbg1(const std::string& msg);
		static void log_dbg2(const std::string& msg);
};

#endif

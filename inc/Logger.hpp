/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 20:49:55 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/20 16:50:48 by fmaurer          ###   ########.fr       */
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
#define BRED "\e[1;91m"
#define BGRN "\e[1;92m"
#define BYLO "\e[1;93m"
#define BPUR "\e[1;94m"
#define BPNK "\e[1;95m"
#define BCYA "\e[1;96m"
#define BGRY "\e[1;97m"
#define BWHT "\e[1;98m"

enum { INFO = 0, DEBUG = 1, BRUTAL = 2 };

#ifndef LOGLEVEL
#define LOGLEVEL INFO
#endif

// IDEA: try to pass a logfile to the Logger-Class via class global variable...
// is this even possible with this utility class?
// ... gotta try this any time soon.
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
		static void log_srv(const std::string& srv_name, const std::string& msg);
		static void log_reqres(const std::string& resreq, const std::string& data);

		// The idead behind log_dbg{0,1,2} goes like this: at compile-time specify
		// `-DLOGLEVEL={0,1,2}` and in the code use debug functions accordingly. so
		// if you want to have the most brutally detailed level of debugging output
		// compile with `-DLOGLEVEL=2`. Then even messages with log_dbg2() will be
		// printed. In LOGLEVEL == DEBUG == 1 log_dbg1 and log_dbg0 will be printed,
		// and in LOGLEVEL == INFO == 0 only log_dbg0 will be shown.
		static void log_dbg0(const std::string& msg);
		static void log_dbg1(const std::string& msg);
		static void log_dbg2(const std::string& msg);
};

#endif

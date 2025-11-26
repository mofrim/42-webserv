/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 20:49:55 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/26 09:18:43 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

// shell colors
#define RED "\e[31m"
#define CYA "\e[36m"
#define YLO "\e[33m"
#define RST "\e[0m"

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
};

#endif

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IWebserv.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zrz <zrz@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 06:16:39 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/04 20:06:40 by zrz              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// This is the interface for the Webserv class

class IWebserv {
	public:
		IWebserv() {};
		IWebserv(const IWebserv& other) { (void)other; };
		IWebserv& operator=(const IWebserv& rhs)
		{
			(void)rhs;
			return (*this);
		};
		virtual ~IWebserv() {};
		virtual void shutdown()	 = 0;
		virtual void run()			 = 0;
		virtual void parse_cfg() = 0;
};

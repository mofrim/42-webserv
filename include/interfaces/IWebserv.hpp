/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IWebserv.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 06:16:39 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/25 08:31:31 by fmaurer          ###   ########.fr       */
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

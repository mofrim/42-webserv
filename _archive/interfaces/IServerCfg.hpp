/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IServerCfg.hpp                                        :+:      :+:    :+:
 */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 07:35:40 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/25 07:48:18 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

class Location;

class IServerCfg {
	public:
		IServerCfg() {};
		IServerCfg(const IServerCfg& other) { (void)other; };
		IServerCfg& operator=(const IServerCfg& rhs)
		{
			(void)rhs;
			return (*this);
		}
		virtual ~IServerCfg() {};
		virtual void setPort(uint16_t port)									= 0;
		virtual void setHost(in_addr_t host)								= 0;
		virtual void setServerName(std::string name)				= 0;
		virtual void setRoot(std::string root)							= 0;
		virtual void setServerAddr(sockaddr_in server_addr) = 0;
		virtual void setListenFd(int listen_fd)							= 0;

		// virtual void setClientMaxBodySize(unsigned long cli_max_body_size) = 0;
		// virtual void setIndex(std::string index)													 = 0;
		// virtual void setLocations(std::vector<Location> locations)				 = 0;

		virtual uint16_t		getPort()				= 0;
		virtual in_addr_t		getHost()				= 0;
		virtual std::string getServerName() = 0;
		virtual std::string getRoot()				= 0;
		virtual sockaddr_in getServerAddr() = 0;
		virtual int					getListenFd()		= 0;

		// virtual unsigned long					getClientMaxBodySize() = 0;
		// virtual std::string						getIndex()						 = 0;
		// virtual std::vector<Location> getLocations()				 = 0;
};

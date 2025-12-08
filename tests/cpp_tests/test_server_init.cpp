/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_server_init.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/08 11:40:47 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/08 13:21:20 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../test-utils.hpp"
#include "Server.hpp"
#include "ServerCfg.hpp"

// TODO: implement this
void test_server_init()
{
	print_test_section_header("Server init test");
	ServerCfg cfg;
	Server		srv(cfg);
}

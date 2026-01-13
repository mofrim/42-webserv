/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_socket.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:06:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/01/13 15:12:39 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../test-utils.hpp"
#include "Socket.hpp"

#include <iostream>
#include <stdexcept>

// test logic goes here, for any failure return (-1) or throw exception.
// otherwise -> success
int _test_socket()
{
	Socket sock0("localhost", 1234);
	sock0.print_addrllist();
	Socket sock1("jeanluc", 1234);
	sock1.print_addrllist();
	Socket sock2("12312.a", 1234);
	sock2.print_addrllist();

	return (0);
}

void test_socket()
{
	int ret = 0;
	print_test_section_header("socket test section header");
	try {
		ret = _test_socket();
	} catch (const std::exception& e) {
		print_test_fail("Test \"test_socket\" failed with following exception:\n"
				+ std::string(e.what()));
	}
	if (ret == -1) {
		print_test_fail("Test \"test_socket\" failed with -1");
	}
}

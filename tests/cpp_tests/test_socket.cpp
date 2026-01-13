/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_socket.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:06:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/01/13 13:44:14 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../test-utils.hpp"

#include <iostream>
#include <stdexcept>

// test logic goes here, for any failure return (-1) or throw exception.
// otherwise -> success
int _test_socket() { return (0); }

void test_socket()
{
	int ret = 0;
	print_test_section_header("socket test section header");
	try {
		ret = _test_socket();
	} catch (const std::exception& e) {
		print_test_fail(
				">> Test \"test_socket\" failed with following exception:\n"
				+ std::string(e.what()));
	}
	if (ret == -1) {
		print_test_fail(">> Test \"test_socket\" failed with -1");
	}
}

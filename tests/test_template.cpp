/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_template.cpp                                  :+:      :+:    :+:   */
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
int _test_template() { return (0); }

void test_template()
{
	int ret = 0;
	print_test_section_header("template test section header");
	try {
		ret = _test_template();
	} catch (const std::exception& e) {
		print_test_fail(
				">> Test \"test_template\" failed with following exception:\n"
				+ std::string(e.what()));
	}
	if (ret == -1) {
		print_test_fail(">> Test \"test_template\" failed with -1");
	}
}

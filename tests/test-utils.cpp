
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 12:37:16 by fmaurer           #+#    #+#             */
/*   Updated: 2025/08/03 19:06:32 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test-utils.hpp"

#include <iostream>
#include <sstream>
#include <string>

void newline() { std::cout << std::endl; }

void print_test_section_header(const std::string& title)
{
	std::cout << "\e[35m[TESTING] __" << title << "__" << "\e[0m" << std::endl;
}

void print_test_topic(const std::string& what)
{
	std::cout << "\e[34m[TEST] " << what << "\e[0m" << std::endl;
}

void print_test_file_header(const std::string& what)
{
	std::cout << "\e[36m[TESTFILE] " << what << "\e[0m" << std::endl;
}

void print_test_success(const std::string& what)
{
	std::cout << "\e[32m[OK]\e[0m " << what << std::endl;
}

void print_test_fail(const std::string& what)
{
	std::cout << "\e[31m[KO]\e[0m " << what << std::endl;
}

// convert an integer to string, as there is no such thing in c++1972 :(
std::string toString(long num)
{
	std::ostringstream oss;
	oss << num;
	return (oss.str());
}

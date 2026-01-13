/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test-utils.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 12:38:44 by fmaurer           #+#    #+#             */
/*   Updated: 2026/01/13 13:22:26 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TEST_UTILS_HPP
#define TEST_UTILS_HPP

#include <string>

void newline();
void print_test_section_header(const std::string& title);
void print_test_topic(const std::string& what);
void print_test_file_header(const std::string& what);
void print_test_success(const std::string& what);
void print_test_fail(const std::string& what);

std::string toString(long num);

#endif

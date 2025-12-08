/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test-utils.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 12:38:44 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/08 11:39:38 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TEST_UTILS_HPP
#define TEST_UTILS_HPP

#include <string>

void				newline();
std::string toString(long num);
void				print_test_section_header(const std::string& title);
void				print_test_topic(const std::string& what);
void				print_test_file_header(const std::string& what);

#endif

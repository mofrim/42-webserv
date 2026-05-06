/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_utils.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 12:38:44 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/04 08:45:50 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

void        nl();
std::string toString(long num);
void        print_test_section_header(const std::string& title);
void        print_test_topic(const std::string& sec, const std::string& what);
void        print_test_result(bool res, const std::string& msg);
void        print_final_result();

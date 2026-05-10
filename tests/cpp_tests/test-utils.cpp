
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

void nl() { std::cout << std::endl; }

void print_test_section_header(const std::string& title)
{
  std::cout << "\e[35m========[ TESTING " << title << " ]========" << "\e[0m"
            << std::endl;
}

void print_final_result_header()
{
  std::cout << "\e[36m========[ FINAL RESULT ]========" << "\e[0m\n"
            << std::endl;
}

void print_test_topic(const std::string& sec, const std::string& what)
{
  std::cout << "\e[34m------=[\e[33m " << sec << "\e[34m " << what
            << " ]=------\e[0m" << std::endl;
}

void print_test_result(bool res, const std::string& msg)
{
  std::string pre;
  if (!res)
    pre = "  \e[31mKO\e[0m";
  if (res)
    pre = "  \e[32mOK\e[0m";

  std::cout << pre << ": " << msg << std::endl;
}

void print_final_result(bool res, const std::string& msg)
{
  std::string pre;
  if (!res)
    pre = "  \e[1;31mKO";
  if (res)
    pre = "  \e[1;32mOK";

  std::cout << pre << ": " << msg << "\e[0m" << std::endl;
}

// convert an integer to string, as there is no such thing in c++1872 :(
std::string toString(long num)
{
  std::ostringstream oss;
  oss << num;
  return (oss.str());
}

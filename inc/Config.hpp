/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 11:11:31 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/18 15:54:31 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "VServerCfg.hpp"

#include <stdexcept>
#include <vector>

// The Basic Desgin Idea here is:
//
// - If webserv program is called without any cmdline arg we launch the default
// config.
// - If webserv is launched with one cmdline arg we will try and parse the file
// given. If successful the _isDefaultCfg flag will be false.

class Config {
  private:
    std::vector<VServerCfg> _cfgs;

  public:
    Config();
    Config(const Config& other);
    Config& operator=(const Config& other);
    ~Config();

    void                           setCfgs(std::vector<VServerCfg> cfgs);
    const std::vector<VServerCfg>& getCfgs() const;
    void                           parseCfgFile(const std::string& fname);

    class EmptyCfgVectorException: public std::logic_error {
      public:
        EmptyCfgVectorException(const std::string& msg);
    };
};

#endif

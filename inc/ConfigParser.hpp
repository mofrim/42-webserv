/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 08:52:11 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/07 15:09:32 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "VServerCfg.hpp"

#include <fstream>
#include <vector>

class ConfigParser {
  private:
    std::ifstream           _cfgfile;
    const str               _cfgFname;
    std::vector<VServerCfg> _vcfgs;
    bool                    _bad;

    ConfigParser();
    ConfigParser(const ConfigParser& other);
    ConfigParser& operator=(const ConfigParser& other);

  public:
    ConfigParser(const str& cfgFname);
    ~ConfigParser();

    void                           parse();
    bool                           bad() const;
    const std::vector<VServerCfg>& getCfgs() const;
};

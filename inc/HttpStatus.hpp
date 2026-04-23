/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStatus.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 16:51:10 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/23 18:54:50 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "typesAndConstants.hpp"

class HttpStatus {
  private:
    HttpStatus();
    HttpStatus(const HttpStatus& other);
    HttpStatus& operator=(const HttpStatus& other);
    ~HttpStatus();

    static str _getTemplateWithErrStr(const str& s);

  public:
    static str getDefaultErrPage(u16 code);
    static str getStatusStr(u16 code);
};

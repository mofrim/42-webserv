/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStatus.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 16:51:10 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/24 17:35:33 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// This is a simple utility class for managing everything around HTTP status
// codes. It is supposed to be quick an dirty bc i don't want to spend to much
// time with it.

#pragma once

#include "WsrvLib.hpp"

// supported Status codes

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

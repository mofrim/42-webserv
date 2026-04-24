/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStatus.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 16:51:10 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/24 17:32:30 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// This is a simple utility class for managing everything around HTTP status
// codes. It is supposed to be quick an dirty bc i don't want to spend to much
// time with it.

#pragma once

#include "WsrvLib.hpp"

// supported Status codes

enum {
  HTTP_200 = 200,
  HTTP_300 = 300,
  HTTP_400 = 400,
  HTTP_404 = 404,
  HTTP_500 = 500,
  HTTP_501 = 501,
  HTTP_502 = 502,
};

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

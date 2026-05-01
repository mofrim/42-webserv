/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReqParse.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 15:05:53 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/01 09:48:56 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "WsrvLib.hpp"

#include <vector>

// https://datatracker.ietf.org/doc/html/rfc9112#section-3
#define MAX_REQLINE_LEN 8000
#define MAX_TARGET_LEN 7886

class ReqParse {
  private:
    ReqParse();
    ReqParse(const ReqParse& other);
    ReqParse& operator=(const ReqParse& other);
    ~ReqParse();

    static std::vector< std::pair<str, str> > _splitHdr(const str& hstr);
    static e_HTTPStatus _readReqline(t_RequestLine& rl, const str& r);

  public:
    static e_HTTPStatus parseReqLine(t_RequestLine& rl, const str& rlstr);
    static e_HTTPStatus parseHeaders(
        std::map<str, str>& _headers, const str& reqstr);
    static int validateUrl(const str& u);
};

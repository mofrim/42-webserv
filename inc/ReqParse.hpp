/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReqParse.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 15:05:53 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/27 15:09:30 by fmaurer          ###   ########.fr       */
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

    static std::vector< std::pair<str, str> > splitHdr(const str& hstr);

  public:
    static int parseReqLine(t_RequestLine& rl, const str& rlstr);
    static int parseHeaders(std::map<str, str>& _headers, const str& reqstr);
    static int validateUrl(const str& u);
};

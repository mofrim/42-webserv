/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReqParse.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 15:05:53 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/23 16:39:15 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "typesAndConstants.hpp"

#define MAX_REQLINE_LEN 8000
#define MAX_TARGET_LEN 7886

class ReqParse {
  private:
    ReqParse();
    ReqParse(const ReqParse& other);
    ReqParse& operator=(const ReqParse& other);
    ~ReqParse();
    typedef enum { RL_METHOD, RL_HTTPVER, RL_TARGET } e_rlStates;

  public:
    static int parseReqLine(t_RequestLine& rl, const str& rlstr);
    static int validateUrl(const str& u);
};

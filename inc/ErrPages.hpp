/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrPages.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 16:51:10 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/20 16:55:01 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "typesAndConstants.hpp"

class ErrPages {
  private:
    ErrPages();
    ErrPages(const ErrPages& other);
    ErrPages& operator=(const ErrPages& other);
    ~ErrPages();

    static str _getTemplateWithErrStr(const str& s);

  public:
    static str getDefaultErrPage(u16 code);
};

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   URL.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/06 22:32:20 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/06 23:00:18 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WsrvLib.hpp"

class URL {
  private:
    str _path;
    str _query;
    str _fragment;

    bool _bad;

  public:
    URL();
    URL(const URL& other);
    URL& operator=(const URL& other);
    ~URL();
    URL(const str& u);

    str sanitizeTargetURL(const str& target);
};

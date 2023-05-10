/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Configuration.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbouhsis <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/10 09:40:37 by hbouhsis          #+#    #+#             */
/*   Updated: 2023/05/10 09:40:41 by hbouhsis         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Http.hpp"
#include "Server.hpp"

class Configuration {
	public :
		std::vector<Server> parse(std::string configFilePath);

	private :
		bool _extractServerConfigLine(std::string line, Server& current_server);
		bool _extractLocationConfigLine(std::string line, Location& current_location);
};



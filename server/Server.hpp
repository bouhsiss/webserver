/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbouhsis <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/10 09:42:14 by hbouhsis          #+#    #+#             */
/*   Updated: 2023/05/10 09:42:15 by hbouhsis         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <unistd.h>
#include "Location.hpp"
//addrinfo
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
//select()
#include <sys/select.h>

class Location;

class Server {
    public :
		std::map<std::string, Location *>& getLocations();
		const std::string& getServerName();
		const std::string& getHost() const;
		const std::string& getPort() const;
		const std::string& getRoot() const;
		const std::string& getIndex() const;
		const std::string& getAutoIndex() const;
		const std::map<int, std::string>& getErrorPage();
		const size_t& getClientBodySizeLimit() const;
		const int& getListenSocket() const;
		
		void setPort(std::vector<std::string> const &tokens);
		void setServerName(std::vector<std::string> const &tokens);
		void setHost(std::vector<std::string> const &tokens);
		void setErrorPage(std::vector<std::string> const &tokens);
		void setClientBodySizeLimit(std::vector<std::string> const &tokens);
		void setRoot(std::vector<std::string> const &tokens);
		void setIndex(std::vector<std::string> const &tokens);
		void setAutoIndex(std::vector<std::string> const &tokens);

		void isServerValid();
		void setServerDefaultValues();
		void setupListenSocket();
		void setDefaultLocation();

		Server();
		~Server();

    private :
		int _listenSocket;

		std::map<std::string, Location *> _Locations;
        std::string _server_name;
		std::string _host;
		std::string _port;
        std::map<int, std::string> _error_page;
        size_t _client_body_size_limit;
		std::string _root;
		std::string _index;
		std::string _autoIndex;
		friend  class Configuration;
};

std::ostream& operator<<(std::ostream &out, Server& c);

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerFarm.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbouhsis <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/10 09:42:35 by hbouhsis          #+#    #+#             */
/*   Updated: 2023/05/10 09:42:36 by hbouhsis         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Configuration.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Http.hpp"
#include "Response.hpp"

#define MIME_TYPES_FILE_PATH "/Users/hbouhsis/Desktop/webserver/conf.d/mime.types"

class Request;
class Response;

class ServerFarm {
	public :
		const std::map<int, Server *>&	getActiveServers() const;
		const std::map<int, Server *>&	getClientSockets() const;
		void							configure(std::string configFilePath);
		static ServerFarm				*getInstance();
		std::vector<Server *>&			getServers();
		void							initServers();
		void							runEventLoop();
		const std::map<std::string, std::string>& getMIMEtypes();
	private :
		ServerFarm(const ServerFarm &other);
		void operator=(const ServerFarm &other);
		ServerFarm();
		~ServerFarm();

		bool							isServerActive(Server &server);
		void							areServersDuplicated();
		void							handleResponse(fd_set *tmpWriteFds);
		void							handleNewClient(fd_set *tmpReadFds, int *fdmax);
		void							handleRequest(fd_set *tmpReadFds);
		void							readMIMEtypes();
		//the singletone server farm class instance
		static ServerFarm				*instancePtr;

		Configuration						_config;
		std::vector<Server *>				_servers;
		std::map<int, Server *>				_activeServers; // a map with the listening socket as key, and a pointer to the socket's server as a value
		std::map<int, Server *>				_clientSockets;
		std::map<int, Response *>			_writeSockets;
		//select() read and write fd set
		fd_set								_readFds;
		fd_set								_writeFds;
		// map to store the mimeTypes
		std::map<std::string, std::string>	_MIMEtypes;
		std::map<std::string, std::string>	_ReverseMIMEtypes;
};	

std::ostream& operator<<(std::ostream &out, ServerFarm& c);

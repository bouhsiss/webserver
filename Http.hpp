/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Http.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbouhsis <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/10 09:41:04 by hbouhsis          #+#    #+#             */
/*   Updated: 2023/05/10 09:41:07 by hbouhsis         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<set>
#include<fstream>
#include<algorithm>
#include<map>
#include<exception>
#include<sys/socket.h>
#include<netdb.h>
#include<cstring>
#include<sstream>

#define DEFAULT_CONFIG_FILE "default.conf"
#define BACKLOG 10

#define RED "\033[1;31m"
#define BLUE "\033[1;34m"
#define GREEN "\033[1;32m"
#define RESET "\033[0m"
#define YELLOW "\033[33m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"

namespace Http {
	// custom exception for config file parsing errors.
	class HttpException : public std::exception {
		private : 
			std::string _message;
		public :
			HttpException(std::string message) : _message(message) {}
			const char *what() const throw() {return(_message.c_str());}
			~HttpException() throw() {}
	};

	class ConfigFileErrorException : public Http::HttpException {
		public :
			ConfigFileErrorException(std::string msg) : Http::HttpException(msg) {}
	};

	class NetworkingErrorException : public Http::HttpException {
		public :
			NetworkingErrorException(std::string msg) : Http::HttpException(msg) {}
	};

	std::vector<std::string> tokenize(std::string const &str, const char* delim);
	void trimSpaces(std::string &line);
	bool strIsNumber(const std::string &s);
	void printAddr(struct addrinfo *peerAddress);
};

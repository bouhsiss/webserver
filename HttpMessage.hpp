/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpMessage.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbouhsis <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/10 09:41:25 by hbouhsis          #+#    #+#             */
/*   Updated: 2023/05/10 09:41:27 by hbouhsis         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Http.hpp"

class HttpMessage {
	public :
		HttpMessage();
		HttpMessage(HttpMessage const& other);
		HttpMessage& operator=(HttpMessage const& other);
		~HttpMessage();

		const std::string& getStartLine() const;
		const std::string& getBody() const;
		const std::map<std::string, std::string>& getHeaders() const;

		void setStartLine(std::string StartLine);
		void setBody(std::string Body);
		void setHeaders(std::string name, std::string value);


		void parse(const std::string& Message);
	protected :
		std::string _StartLine;
		std::map<std::string, std::string> _Headers;
		std::string _Body;
};














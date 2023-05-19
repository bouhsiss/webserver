/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbouhsis <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/10 09:41:36 by hbouhsis          #+#    #+#             */
/*   Updated: 2023/05/10 09:41:38 by hbouhsis         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once 
#include"Http.hpp"
#include "Server.hpp"

class Server;

class Location {
    public :
		Location();
		~Location(){}

		std::vector<std::string>& getAllowedMethods();
		std::map<std::string, std::string>& getCgiPath() const;
		std::string& getPath() const;
		std::string& getRedirect() const;
		std::string& getRoot() const;
		std::string& getAutoIndex() const;
		std::string& getIndex();
		std::string& getUploadPath() const;
		std::string& getCgiExtension() const;

		void setPath(std::vector<std::string> const &tokens);
		void setAllowedMethods(std::vector<std::string> const &tokens);
		void setRedirect(std::vector<std::string> const &tokens);
		void setRoot(std::vector<std::string> const &tokens);
		void setAutoIndex(std::vector<std::string> const &tokens);
		void setIndex(std::vector<std::string> const &tokens);
		void setUploadPath(std::vector<std::string> const &tokens);
		void setCgiExtension(std::vector<std::string> const &tokens);
		void setCgiPath(std::vector<std::string> const &tokens);

		void isLocationComplete(Server &parentServer);
    private :
        std::string _path;
        std::vector<std::string> _allowed_methods;
        std::string _redirect;
        std::string _root;
        std::string _autoindex;
        std::string _index;
        std::string _upload_path;
        std::string _cgi_extension;
		std::map<std::string, std::string> _cgi_path;

		friend class Configuration;
};

std::ostream& operator<<(std::ostream &out, Location &c);

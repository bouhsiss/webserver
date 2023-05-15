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
class Location {
    public :
		Location();
		~Location(){}

		const std::string getPath() const;
		const std::vector<std::string> getAllowedMethods();
		const std::string getRedirect() const;
		const std::string getRoot() const;
		const std::string getAutoIndex() const;
		const std::string getIndex();
		const std::string getUploadPath() const;
		const std::string getCgiExtension() const;
		const std::string getCgiPath() const;

		void setPath(std::vector<std::string> const &tokens);
		void setAllowedMethods(std::vector<std::string> const &tokens);
		void setRedirect(std::vector<std::string> const &tokens);
		void setRoot(std::vector<std::string> const &tokens);
		void setAutoIndex(std::vector<std::string> const &tokens);
		void setIndex(std::vector<std::string> const &tokens);
		void setUploadPath(std::vector<std::string> const &tokens);
		void setCgiExtension(std::vector<std::string> const &tokens);
		void setCgiPath(std::vector<std::string> const &tokens);

		void isLocationValid();
    private :
        std::string _path;
        std::vector<std::string> _allowed_methods;
        std::string _redirect;
        std::string _root;
        std::string _autoindex;
        std::string _index;
        std::string _upload_path;
        std::string _cgi_extension;
        std::string _cgi_path;

		friend class Configuration;
};

std::ostream& operator<<(std::ostream &out, Location &c);

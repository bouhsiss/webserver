# webserver

--> the project requirements :
	- the program must take a config file as argument
	- your server must never block and the client can be bounced properly if necessary
	- it must be non-blocking and use only 1 poll() (or equivalent) for all the I/O operations between the client and the server (listen included)
	- poll() or equivalent must check read and write at the same time 
	- you must never do a read or a write operation without going through poll() (or equivalent)
	- checking the value of errno is strictly forbidden after a read or a write operation
	- you don't need to use poll() (or equivalent) before reading your configuration file.
	- because you have to use a non-blocking file descriptors, it is possible to use read/recv or write/send function with no poll() (or equivalent), and your server wouldn't be blocking. but it would consume more system ressources. thus it's not allowed to read/recv or write/send in any file descriptor without using poll()
	- you can use every macro and define like FD_SET, FD_CLR, FD_ISSET, FD_ZERO (understanding what and how they do it is very useful)
	- a request to your server should never hang forever
	- you can consider that NGINX is HTTP 1.1 compliant and may be used to compare headers and answer behaviours.
	- your HTTP response status codes must be accurate
	- your server must have defeault error pages of none are provided
	- you can't use fork for something else than CGI (like php, or python and so forth)
	- you must be able to serve a fully static website
	- clients must be able to upload files
	- you need at least GET, POST, and DELETE methods
	- stress test your server, it must stay available at all cost
	- your server must be able to listen to multiple ports

--> the configuration file requirements :
	- choose the port and host for each 'server'
	- setup the server_names or not
	- the first server for a host:port will be default for this host:port (that means it will answer to all requests that don't belong to an other server)
	- setup default error pages
	- limit client body size
	- setup routes with one or multiple of the following rules/configuration (routes won't be using regexp) : 
			* define a list of accepted HTTP methods for the route
			* define a HTTP redirection
			* define a directory or a file from where the file should be searched (for example, if url /kapouet is rooted to /tmp/www, url  /kapouet/pouic/toto/pouet is /tmp/www/pouic/toto/pouet)
			* turn on or off directory listing
			* set a default file to answer if the request is a directory
			* execute CGI based on certain file extension (for example .php)
			* make it work with POST and GET methods
			* make the route able to able to accept uploaded files and configure where they should be saved
				
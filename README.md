# HTTP Web Server in C++
This is a lightweight HTTP web server implemented from scratch in C++. This web server is designed to handle HTTP requests, parse configuration files, and serve static content. It supports various HTTP methods, including GET, POST, and DELETE. The server also incorporates concurrent connection handling using the select system call, allowing for efficient multiplexing of multiple client requests.

## Features
 * Parse configuration files to customize server settings.
 * Efficiently handle concurrent connections and multiplexing using select.
 * Support for GET, POST, and DELETE HTTP methods.
 * Execute CGI based on [.php, .py] file extension.
 * Serve a fully static website.
 * have default error pages if none are provided.

 ## Getting started
  To get started with this HTTP web server, follow these steps:

  Clone the repository:

  ```
    git clone https://github.com/yourusername/your-http-server.git
    cd your-http-server
  ```

  Build the server:

  ```
  make
  ```

  Start the server:

  ```
  ./webserver [config file] (config file is optional. it takes conf.d/default.conf by default)
  ```

  Access the server by opening a web browser and navigating to `http://localhost:80` (by default).

  ## Configuration
  The server can be configured by specifying a custom configuration file by providing the config file p when starting the server, or by editing the default configuration file located at conf.d/default.conf. The configuration file allows you to specify various settings for the server. Here are some key configuration options:

 * Choose the port and host of each server.
 * Setup server names or leave them unset.
 * The first server for a host:port will be the default for this host:port.
 * Configure default error pages.
 * Limit client body size.
 * Define routes with rules such as accepted HTTP methods, redirection, directory or file location, directory listing, error pages, CGI execution, handling POST and GET methods, and handling uploaded files.

For detailed information on how to set up the configuration file, refer to the commented section provided in the default configuration file located at conf.d/default.conf.

  


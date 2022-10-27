# webserv
### A 42 project

A simple HTTP 1.1 web server written in C++.
The aim of this project is to learn the theory behind network programming: only a few C functions related to socket programming were allowed, so tht we had to face the HTTP parsing and response handeling ourselves. The multiplexing part is handeled using poll() for siplicity; we don't use threads.  
The structur of the configuration file is inspired by nginx's. A default configuration is provided if no file is selected when starting the sever.

## Usage

At the root of the repository:
```
make
./webserv [ path of config file ]
```

### Configuration

You can configure multiple servers, serving different pages on different ports.  
To configure one server, open a server block like so:

```
server {
}
```
Inside this block, you have to provide at least a host, a port, the root of your webpage's directory and the index file.
Example configuration:

```
server {
	host: localhost;
	port:	5555;
	root:	./example_var/example_www;
	index:  index.html;
	server_name: exampleserver.awesome;
	error: ./default_pages/errors/;
	limit_body: 1024;
	http_methods: GET,  POST;
	http_redirects: non;
	directory_listing: 		off;
	download: ./default_pages/downloads/;
	cgi: non;
}
```

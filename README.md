# webserv
### A 42 project

A simple HTTP 1.1 web server written in C++.
The aim of this project is to learn the theory behind network programming: only a few C functions related to socket programming were allowed, so tht we had to face the HTTP parsing and response handeling ourselves. The multiplexing part is handeled using poll() for siplicity; we don't use threads.  
We set up our own simple config structure based on the requirements. A default configuration is provided if no file is selected when starting the sever.

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
Inside this block, you have to provide at least a host, a port, the root of your webpage's directory and the index file. Each field have to contain a value. If the functionality is optional but not requered the non or off world has to be there.
--> cgi: .php; (the webpage contain php code)
--> cgi: non; (the webpage don't run any cgi)
--> limit_body: -1; (no limit)

Example configuration (with minimal requered fields): 

```
server {
	host: localhost;
	port: 4242;
	root: ./example_sites/http;
	index: index.html;
	server_name: non;
	error: non;
	limit_body: -1;
	http_methods: GET;
	http_redirects: non;
	location: [];
	directory_listing: off;
	download: non;
	cgi: non;
}
```

Example configuration (with all fields set with correct syntax): 
([...] ... [...]: meaning that multiple location blockes can be fused one after each other but obviously they have to be filled up)

```
server {
	host: localhost;
	port: 5555;
	root: ./example_var/example_www;
	index: index.php;
	server_name: exampleserver.awesome;
	error: 501:/error/501.html, 400:/error/400.html;
	limit_body: 1024;
	http_methods: GET, POST;
	http_redirects: https://en.wikipedia.org/wiki/Quantum_mechanics;
	directory_listing: on;
	download: ./downloads;
	cgi: .php;
	location: [
				folder: http;
				root:	./example_var/something/;
				index:  index.html;
				http_methods: GET;
				http_redirects: non;
				directory_listing: off;
				download: non;
				cgi: non;
			]
			[...] ... [...]
			;
}
```

### Cgi

Our server suports two Common Gateway Interface (cgi): php and perl. For php-cgi we use the standard binary of php-cgi. In the case of perl we wrote a simple perl-cgi on our own. What will run the code in perl. It suports embed perl in html code.

#include "../include/CGI.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <iostream>
#include <iostream>
#include <fstream>  
#include <fstream>
#include <sstream>
#include <cctype>
#include <stdlib.h>

 #include <fcntl.h>

namespace ws {

	Cgi::Cgi() {}
	Cgi::~Cgi() {}

	void Cgi::readHTML()
	{
		std::ifstream confFile;
		confFile.open("/example_sites/phptestsite/index.html", std::ios::in);
		if (confFile.fail())
			return ;
		std::ofstream outfile ("response.html"); // how to what to do with the created expanded html is it stored permanently in a temp folder or override teh orgiinal one?
		if (outfile.fail())
			return ;
		char tempChar;
		// temp[128] = '\0';
		// outfile.eof(); // true if teh file is finished. 
		std::string buffer;
		std::string::size_type checkChunk = std::string::npos;
		while (!(outfile.eof()))
		{
			while (checkChunk == std::string::npos)
			{}

				confFile >> tempChar;
				buffer += tempChar;
				if (tempChar == '?' && confFile.peek() == '>')
				{
					confFile >> tempChar;
					buffer += tempChar;
					outfile << findPHPtag(buffer);
					buffer.clear();
				}
			
			// faitly logic...
		}
	}

	std::string & Cgi::findPHPtag(std::string const & htmLine)
	{
		std::string::size_type phpindex;
		std::string::size_type endphpindex;
		phpindex = htmLine.find("<?php");
		if (phpindex == std::string::npos)
			return (htmLine); //not usre jet but means is over.
		endphpindex = htmLine.find("?>");
		if (endphpindex ==	std::string::npos)
			return (htmLine); // throw some error that php is wrong
		return ( executeCgi( createTempPHP( htmLine.substr(phpindex, endphpindex - phpindex) ) ) );
	}


	std::string & Cgi::createTempPHP(std::string const & phpCode) 
	{
		std::string temp = "temp.php";

		std::ofstream outfile (temp);
		outfile << phpCode;
		outfile.close();
		return (temp);
	}


	std::string & Cgi::executeCgi(std::string const & phpfile) 
	{
		int id;
		int fd[2];
		std::string temp;

		pipe(fd);
		id = fork();
		if (id == 0)
		{
			dup2(fd[1],STDOUT_FILENO);
					close(fd[1]);
					close(fd[0]);
			execl("/usr/bin/php", "php", phpfile.c_str(), NULL);
			return (temp);
		}
		else
		{
			waitpid(-1, NULL, 0);
			char hold;
			fcntl(fd[0], F_SETFL, O_NONBLOCK);
			while (read(fd[0], &hold, 1) != -1)
				temp += hold;
			close(fd[1]);
			close(fd[0]);
			return (temp);
		}
	}
}

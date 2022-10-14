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

	void Cgi::runRawPHP(std::string const & phpFilePath)
	{
		executeCgi(phpFilePath);
	}
	
	void Cgi::readHTML(std::string const & htmlFilePath)
	{
		std::ifstream confFile;
		confFile.open(htmlFilePath, std::ios::in);
		if (confFile.fail())
			return ;
		std::ofstream outfile; // how to what to do with the created expanded html is it stored permanently in a temp folder or override teh orgiinal one?
		outfile.open("response.html", std::ios_base::app);

		// insted of an outfil it is possible to send the data directly to the client.
		if (outfile.fail())
			return ;
		char tempChar;
		std::string buffer;
		// int i = 0;
		bool commentflag = false;
		confFile >> std::noskipws;
		while (!(confFile.eof()))
		{
			confFile >> tempChar;
			if (commentflag == false && tempChar == '<' && confFile.peek() == '?')
			{
				outfile << buffer;
				buffer.clear();
			}
			else if (commentflag == false && tempChar == '<' && confFile.peek() == '!')
			{
				buffer += tempChar;
				confFile >> tempChar;
				buffer += tempChar;
				confFile >> tempChar;
				if (tempChar == '-' && confFile.peek() == '-')
				{
					buffer += tempChar;
					confFile >> tempChar;
					commentflag = true;
					// buffer += tempChar;
				}
			}
			else if (commentflag == true && tempChar == '-' && confFile.peek() == '-')
			{
					buffer += tempChar;
					confFile >> tempChar;
					buffer += tempChar;
					confFile >> tempChar;
					if (tempChar == '>')
						commentflag = false;
			}
			buffer += tempChar;
			if (commentflag == false && tempChar == '?' && confFile.peek() == '>')
			{
				confFile >> tempChar;
				buffer += tempChar;
				outfile << findPHPtag(buffer);
				buffer.clear();
			}
		}
		// ----------Large file could cose troubles here??------------
		buffer.pop_back();
		outfile << buffer;
		buffer.clear();
	}

	std::string const Cgi::findPHPtag(std::string const & htmLine)
	{
		std::string::size_type phpindex;
		std::string::size_type endphpindex;
		phpindex = htmLine.find("<?php");
		if (phpindex == std::string::npos)
			return (htmLine); //not usre jet but means is over.
		endphpindex = htmLine.find("?>");
		if (endphpindex ==	std::string::npos)
			return (htmLine); // throw some error that php is wrong
		return ( executeCgi( createTempPHP( htmLine.substr(phpindex, endphpindex - phpindex + 2) ) ) );
	}


	std::string Cgi::createTempPHP(std::string const & phpCode) 
	{
		std::string temp = "temp.php";

		std::ofstream outfile (temp);
		outfile << phpCode;
		outfile.close();
		return (temp);
	}


	std::string Cgi::executeCgi(std::string const & phpfile) 
	{
		int id;
		int fd[2];
		std::string temp;

		char **env; // GET this one has a QUERY_STRING=?shjdhfgjh=dgdfgdg&fgdfgdfgd=dfgdf
	// POST
	// body --> ?dfgdfgdg=dgdfgdf&dgdfg=dgdg?
	//php-cgi REQEST_METHOD=POST;
	// env[] = "PATH_TERM=./cgi.php"
	execve("php-cgi", NULL, env);

		pipe(fd);
		id = fork();
		if (id == 0)
		{
			dup2(fd[1],STDOUT_FILENO);
					close(fd[1]);
					close(fd[0]);
			if (phpfile.back() == 'p')
				execl("/usr/bin/php", "php", phpfile.c_str(), NULL);
				// [ + ] execve + env TODO
			else
				execl("/usr/local/bin/perl", "perl", phpfile.c_str(), NULL);
			exit(0);
			// throw error....
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
		}
		return (temp);
	}
}

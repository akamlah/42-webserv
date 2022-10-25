#include "../include/CGI.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <iostream>
#include <iostream>
#include <fstream>  
#include <fstream>
#include <sstream>
#include <cctype>
#include <stdlib.h>
#include <stdio.h>

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

	// void fillenv() X-Powered-By: PHP/7.4.21
	// close shod down both end... so client maybe never get the alst response suse shoot down...
	// {

	// }

	std::string Cgi::executeCgiNew(char **env) 
	{
		// // manual test
		std::string temp;
		// temp = "X-Powered-By: PHP/7.4.21 Cache-Control: no-cache Content-type: text/event-stream;charset=UTF-8\r\n\r\ndata: New random number: 871\r\n\r\n";
		// std::cout << env[0] << std::endl;
		int id;

		FILE * bobout = tmpfile();
		int outy = fileno(bobout);
		char **test = new char*[2];
		test[0] = &(*((new std::string("php-cgi")))->begin()); 
		test[1] = NULL;

		for (size_t i = 0; env[i] != NULL; i++)
		{	
			std::cout << env[i] << "\n";
			/* code */
		}
		
		id = fork();
		if (id == 0)
		{
			FILE * bobin = tmpfile();
			int inty = fileno(bobin);
			int testlegnth = 0;
			if (env[0])
				testlegnth = strlen(env[0]);
			if (testlegnth > 0)
			{
				// int boby  = open("./look.txt",O_RDWR );
				write(inty, env[0], strlen(env[0]));
				// write(boby, env[0], strlen(env[0]));
				// close(boby);
			}
			if (testlegnth > 0 && lseek(inty, 0, SEEK_SET) == -1)
			{
				std::cerr << "Something is not rigth with the data in fiel\n";
			}
			else
			{
				if (dup2(inty ,STDIN_FILENO) == -1)
				{
					std::cerr << "Error dup2 std IN\n";
				}
			}
			if (dup2(outy ,STDOUT_FILENO) == -1)
			{
				std::cerr << "Error dup2 std OUT\n";
			}
			// rewind(bobin);
			fclose(bobin);
			close(outy);
			close(inty);
			if (execve("./CGI/php-cgi",test , env) == -1 )
				std::cerr << "Error in cgi Execution\n";
			delete [] test;
			exit(0);
			// throw error....
		}
		else
		{
			waitpid(-1, NULL, 0);
			char hold;
			int outy = fileno(bobout);

			rewind(bobout);
			while (read(outy, &hold, 1) > 0)
				temp += hold;
			close(outy);
			fclose(bobout);

		}
		delete [] test;
		return (temp);
	}


	std::string Cgi::executeCgi(std::string const & phpfile) 
	{
		int id;
		int fd[2];
		std::string temp;
		char ** env;
		env = new char*[7];

		env[0] = strdup("REQUEST_METHOD=POST");
		env[1] = &(*(std::string("PATH_TRANSLATED=" + phpfile).begin()));
		// env[1] = &(*((new std::string("PATH_TRANSLATED=" + phpfile))->begin()));
		// env[1] = strdup(("PATH_TRANSLATED=" + phpfile).c_str());
		// env[1] = strdup("PATH_TRANSLATED=./example_sites/phptestsite/send_sse.php");
		env[2] = strdup("SERVER_PORT=8100");
		env[3] = strdup("CONTENT_LENGTH=0");
		env[4] = strdup("QUERY_STRING=fname=SISI");
		env[5] = strdup("REDIRECT_STATUS=200");
		env[6] = NULL;


		pipe(fd);
		id = fork();
		if (id == 0)
		{
			dup2(fd[1],STDOUT_FILENO);
					close(fd[1]);
					close(fd[0]);
			if (phpfile.back() == 'p')
			{
					if (execve("./CGI/php-cgi",NULL , env) == -1 )
						std::cerr << "Error in cgi Execution\n";
				// execl("/usr/bin/php", "php", phpfile.c_str(), NULL);
			}
				// [ + ] execve + env TODO
			else
				if (execl("/usr/local/bin/perl", "perl", phpfile.c_str(), NULL) == -1)
					std::cerr << "Error in cgi Execution\n";
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

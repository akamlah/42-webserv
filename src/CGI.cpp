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

	void Cgi::runRaw_perl(std::string const & perlFilePath)
	{
		executeCgi_perl(perlFilePath);
	}
	void Cgi::readHTML(std::string const & htmlFilePath)
	{
		std::ifstream confFile;
		confFile.open(htmlFilePath, std::ios::in);
		if (confFile.fail())
			return ;
		std::ofstream outfile;
		outfile.open("response.html", std::ios_base::app);
		if (outfile.fail())
			return ;
		char tempChar;
		std::string buffer;
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
				outfile << find_perl_tag(buffer);
				buffer.clear();
			}
		}
		buffer.pop_back();
		outfile << buffer;
		buffer.clear();
	}

	std::string const Cgi::find_perl_tag(std::string const & htmLine)
	{
		std::string::size_type Perlindex;
		std::string::size_type endPerlindex;
		Perlindex = htmLine.find("<?perl");
		if (Perlindex == std::string::npos)
			return (htmLine); //not usre jet but means is over.
		endPerlindex = htmLine.find("?>");
		if (endPerlindex ==	std::string::npos)
			return (htmLine); // throw some error that php is wrong
		return ( executeCgi_perl( createTemp_perl( htmLine.substr(Perlindex, endPerlindex - Perlindex + 2) ) ) );
	}

	std::string Cgi::createTemp_perl(std::string const & perlCode) 
	{
		std::string temp = "temp.pl";
		std::ofstream outfile (temp);
		outfile << perlCode;
		outfile.close();
		return (temp);
	}

	std::string Cgi::executeCgiNew(char **env) 
	{
		std::string temp;
		int id;

		FILE * bobout = tmpfile();
		int outy = fileno(bobout);
		char **test = new char*[2];
		test[0] = &(*((new std::string("php-cgi")))->begin()); 
		test[1] = NULL;
		#if DEBUG
				for (size_t i = 0; env[i] != NULL; i++)
				{	
					std::cout << env[i] << "\n";
				}
		#endif
		id = fork();
		if (id == 0)
		{
			FILE * bobin = tmpfile();
			int inty = fileno(bobin);
			int testlegnth = 0;
			if (env[0])
				testlegnth = strlen(env[0]);
			if (testlegnth > 0)
				write(inty, env[0], strlen(env[0]));
			if (testlegnth > 0 && lseek(inty, 0, SEEK_SET) == -1)
				std::cerr << "Something is not rigth with the data in fiel\n";
			else
			{
				if (dup2(inty ,STDIN_FILENO) == -1)
					std::cerr << "Error dup2 std IN\n";
			}
			if (dup2(outy ,STDOUT_FILENO) == -1)
				std::cerr << "Error dup2 std OUT\n";
			fclose(bobin);
			close(outy);
			close(inty);
			if (execve("./CGI/php-cgi",test , env) == -1 ) {
				std::cerr << "Error in cgi Execution\n";
			}
			delete [] test;
			exit(EXIT_FAILURE);
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


	std::string Cgi::executeCgi_perl(std::string const & phpfile) 
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
			if (execl("/usr/bin/perl", "perl", phpfile.c_str(), NULL) == -1 )
				std::cerr << "Error in perl cgi Execution\n";
			exit(0);
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

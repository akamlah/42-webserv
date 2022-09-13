#include "../include/CGI.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <iostream>

#include <fstream>
#include <sstream>
#include <cctype>
#include <stdlib.h>

 #include <fcntl.h>

namespace ws {

	Cgi::Cgi() {}
	Cgi::~Cgi() {}

	std::string Cgi::executeCgi(std::string const & phpfile) 
	{
		int id;
		int fd[2];

		pipe(fd);
		id = fork();
		if (id == 0)
		{
			dup2(fd[1],STDOUT_FILENO);
					close(fd[1]);
					close(fd[0]);
			execl("/usr/bin/php", "php", phpfile.c_str(), NULL);
			return nullptr;
		}
		else
		{
			waitpid(-1, NULL, 0);
			std::string temp;
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

#ifndef __LUKS_HPP__
#define __LUKS_HPP__

#include "config.h"

bool luksOpen(const std::string& dev, const std::string& password, const std::string& dmname)
{
	bool ret = false;

	// popen crashes for no reason (on actually a following fprintf), 
	// so using 'old school'

	int pipes[2];

	pipe(pipes);

	int pid = fork();

	if ( pid == 0 ) 
	{
		close(0); // stdin
		close(pipes[1]);
		dup2(pipes[0], 0); // duplicate read-end of pipe to stdin

		execl( 	cryptsetup, 
			cryptsetup, 
			"luksOpen", 
			dev.c_str(), 
			dmname.c_str(), 
			NULL
			);
		exit(-1);
	}
	else
	{
		write(pipes[1], password.c_str(), password.size());
		write(pipes[1], "\n", 1);
		close(pipes[1]); 

		int st;
		pid_t p = waitpid(pid, &st, 0);

		if ( WIFEXITED(st) && WEXITSTATUS(st) == 0 )
			ret = true;

		close(pipes[0]);
	}

	return ret;
}

bool luksChangeKey(const std::string& dev, const std::string& oldPassword, const std::string& newPassword)
{
	bool ret = false;

	// popen crashes for no reason (on actually a following fprintf), 
	// so using 'old school'

	int pipes[2];

	pipe(pipes);

	int pid = fork();

	if ( pid == 0 ) 
	{
		close(0); // stdin
		close(pipes[1]);
		dup2(pipes[0], 0); // duplicate read-end of pipe to stdin

		execl( 	cryptsetup, 
			cryptsetup, 
			"luksChangeKey", 
			dev.c_str(), 
			NULL
			);
		exit(-1);
	}
	else
	{
		write(pipes[1], oldPassword.c_str(), oldPassword.size());
		write(pipes[1], "\n", 1);
		write(pipes[1], newPassword.c_str(), newPassword.size());
		write(pipes[1], "\n", 1);
		write(pipes[1], newPassword.c_str(), newPassword.size());
		write(pipes[1], "\n", 1);
		close(pipes[1]); 

		int st;
		pid_t p = waitpid(pid, &st, 0);

		if ( WIFEXITED(st) && WEXITSTATUS(st) == 0 )
			ret = true;

		close(pipes[0]);
	}

	return ret;
}

bool luksFormat(const std::string& dev, const char* keySizeBits, const std::string& newPassword)
{
	bool ret = false;

	// popen crashes for no reason (on actually a following fprintf), 
	// so using 'old school'

	int pipes[2];

	pipe(pipes);

	int pid = fork();

	if ( pid == 0 ) 
	{
		close(0); // stdin
		close(pipes[1]);
		dup2(pipes[0], 0); // duplicate read-end of pipe to stdin

		execl( 	cryptsetup, 
			cryptsetup,
			"--cipher",
			"aes",
			"--key-size",
			keySizeBits,
			"luksFormat", 
			dev.c_str(), 
			NULL
			);
		exit(-1);
	}
	else
	{
		write(pipes[1], "YES\n", 4);

		write(pipes[1], newPassword.c_str(), newPassword.size());
		write(pipes[1], "\n", 1);
		write(pipes[1], newPassword.c_str(), newPassword.size());
		write(pipes[1], "\n", 1);
		close(pipes[1]); 

		int st;
		pid_t p = waitpid(pid, &st, 0);

		if ( WIFEXITED(st) && WEXITSTATUS(st) == 0 )
			ret = true;

		close(pipes[0]);
	}

	return ret;
}


#endif
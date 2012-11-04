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

bool luksClose(const std::string& dmname)
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
			"luksClose", 
			dmname.c_str(), 
			NULL
			);
		exit(-1);
	}
	else
	{
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

bool volFormat(const std::string& device, const char *keySize, const std::string& password, const std::string& dmTmpName)
{
	bool ret = false;

	L("volFormat: trying to format %s, key size %s,  tmp devname %s\n", 
			device.c_str(), keySize, dmTmpName.c_str());

	if ( luksFormat ( device, keySize, password ) ) 
	{
		L("luksFormat is ok");

		if ( luksOpen ( device, password, dmTmpName ) ) 
		{
			L("luksOpen is ok");

			std::string cmd = dmFormatCmd + dmTmpName;

			L("Trying to execute %s\n", cmd.c_str());

			if ( system( cmd.c_str() ) == 0 ) 
			{
				L("And it is ok!");
				ret = true;
			}
			else
			{
				L("And it's failed!");
			}

			luksClose ( dmTmpName );
		}
		else
		{
			L("luksOpen failed");
		}
	}
	else
	{
		L("luksFormat failed");
	}

	return ret;
}

#endif

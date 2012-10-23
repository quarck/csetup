#ifndef __KEYBOARD_DEVICE_HPP__
#define __KEYBOARD_DEVICE_HPP__ 

#include <cerrno>
#include <cstddef>

#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>

#include <linux/fb.h>
#include <unistd.h>

#include <sys/select.h>

#include "input.h"

#include "IEventDevice.hpp"

class KeyboardDevice 
	: public IEventDevice 
{
	int m_keyboardFd;

public:
	virtual void onKeyDown(int code) = 0;
	
	virtual void onKeyUp(int code) = 0;

	inline KeyboardDevice(const char *touchDev)
		: m_keyboardFd ( -1 )
	{
		m_keyboardFd = open(touchDev, O_RDONLY);
	}

	bool isValid() const
	{
		return m_keyboardFd != -1;
	}

	int getReadFD()
	{
		return m_keyboardFd;
	}

	int getWriteFD()
	{
		return -1;
	}

	int getExcpFD() 
	{
		return -1;
	}

	void onFDReadReady()
	{
		input_event ev;
		if ( read(m_keyboardFd, &ev, sizeof(ev)) != sizeof(ev))
		{
			close(m_keyboardFd);
			m_keyboardFd = -1;
			return;
		}

		switch (ev.type ) 
		{
		case EV_SYN: 
			switch (ev.code)
			{
			case SYN_REPORT:
				// ignore
				break;
			}
			break;
		case EV_KEY:
			if ( ev.value != 0 ) 
				onKeyDown(ev.code);
			else
				onKeyUp(ev.code);
			break;
		default:
			break;

		}
	}

	void onFDWriteRead()
	{
	}

	void onFDException()
	{
	}
};

#endif

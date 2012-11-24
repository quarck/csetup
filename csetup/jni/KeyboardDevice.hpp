/*
 * Copyright (c) 2012, Sergey Parshin, qrck@mail.ru
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */ 

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

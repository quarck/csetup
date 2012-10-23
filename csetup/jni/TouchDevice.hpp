#ifndef __TOUCH_DEVICE_HPP__
#define __TOUCH_DEVICE_HPP__

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

class TouchDevice 
	: public IEventDevice 
{
	input_absinfo m_absx;
	input_absinfo m_absy;

	int m_touchFd;

	int m_x;
	int m_y;

	int m_xRes;
	int m_yRes;

	bool m_touchActive;
	bool m_prevTouchActive;

public:
	virtual void onTouchDown(int x, int y) = 0;

	virtual void onTouchUpdate(int x, int y) = 0;

	virtual void onTouchUp(int x, int y) = 0;

	inline TouchDevice(const char *touchDev, int xRes, int yRes)
		: m_touchFd ( -1 )
		, m_x ( 0 )
		, m_y ( 0 )
		, m_xRes ( xRes )
		, m_yRes ( yRes )
		, m_touchActive ( false )
		, m_prevTouchActive ( false )
//		, m_shouldQuit ( false )
	{
		m_touchFd = open(touchDev, O_RDONLY);

		if ( m_touchFd != -1 )
		{
			int rx = ioctl(m_touchFd, EVIOCGABS(ABS_MT_POSITION_X), &m_absx);

			int ry = ioctl(m_touchFd, EVIOCGABS(ABS_MT_POSITION_Y), &m_absy);

			if ( rx != 0 || ry != 0 ) 
			{
				close(m_touchFd);
				m_touchFd = -1;
			}
		}
	}

	bool isValid() const
	{
		return m_touchFd != -1;
	}

	int getReadFD()
	{
		return m_touchFd;
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
		if ( read(m_touchFd, &ev, sizeof(ev)) != sizeof(ev))
		{
			close(m_touchFd);
			m_touchFd = -1;
			return;
		}

		switch (ev.type ) 
		{
		case EV_SYN: 
			switch (ev.code)
			{
			case SYN_REPORT:
				report();
				break;
			case SYN_MT_REPORT:
				break;
			}
			break;
		case EV_ABS:
			switch (ev.code) 
			{
			case ABS_X:
				m_x = m_xRes * (ev.value -m_absx.minimum) / (m_absx.maximum - m_absx.minimum + 1);
				m_touchActive = true;
				break;
			case ABS_Y:
				m_y = m_yRes * (ev.value -m_absy.minimum) / (m_absy.maximum - m_absy.minimum + 1);
				m_touchActive = true;
				break;
			case ABS_MT_SLOT:
				break;
			case ABS_MT_POSITION_X:
				m_x = m_xRes * (ev.value -m_absx.minimum) / (m_absx.maximum - m_absx.minimum + 1);
				m_touchActive = true;
				break;
			case ABS_MT_POSITION_Y:
				m_y = m_yRes * (ev.value -m_absy.minimum) / (m_absy.maximum - m_absy.minimum + 1);
				m_touchActive = true;
				break;
			case ABS_MT_PRESSURE:
				if ( ev.value == 0 )
					m_touchActive = 0;
				break;
			case ABS_MT_TOUCH_MAJOR:
				if ( ev.value == 0 ) 
					m_touchActive = 0;					
				break;
			case ABS_MT_TOUCH_MINOR:
				break;
				
			default:
				break;

			}
			break;
		case EV_KEY:
			switch (ev.code)
			{
			case BTN_TOUCH: 
				m_touchActive = ev.value ? true : false;
				break;
			}
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

	inline void report()
	{
		if ( m_touchActive != m_prevTouchActive ) 
		{
			if ( m_touchActive )
			{
				onTouchDown(m_x, m_y);
			}
			else
			{
				onTouchUp(m_x, m_y);
			}
		}
		else
		{
			onTouchUpdate(m_x, m_y);
		}

		m_prevTouchActive = m_touchActive;
	}
};

#endif

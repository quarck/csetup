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

#ifndef __UIMANAGER_HPP__
#define __UIMANAGER_HPP__

#include "PM.hpp"
#include "SleepTimeoutManager.hpp"

class UIManager 
	: public TouchDevice
	, public KeyboardDevice 
{
	UIPane*  m_activePane;
	IWidget* m_activeButton;

	FrameBuffer* m_fb;

	CSleepTimeoutManager 	m_sleepTimeoutManager;

	bool m_shouldQuit;

public:
	inline UIManager(
				const char *touchDev, 
				const char *kbdDev, 
				FrameBuffer* fb
			)
		: TouchDevice ( touchDev, fb->xres(), fb->yres() )
		, KeyboardDevice ( kbdDev )
		, m_fb (fb)
		, m_activeButton ( NULL )
	{
	}

	bool isValid() 
	{
		return this->TouchDevice::isValid() && this->KeyboardDevice::isValid();
	}

	void setShouldQuit()
	{
		m_shouldQuit = true;
	}


	void setSleepTimeout(int pTimeout)
	{
		m_sleepTimeoutManager.setSleepTimeout(pTimeout);	
	}

	int getSleepTimeout() const
	{
		return m_sleepTimeoutManager.getSleepTimeout();
	}


	void onKeyDown(int code)
	{
		if ( code == KEY_POWER )
		{
		}
	}
	
	void onKeyUp(int code)
	{
		if ( code == KEY_POWER )
		{
			CPM::shutdown();
		}
	}

	void onTouchDown(int x, int y)
	{
		Point pt(x,y);

		if ( m_activeButton ) 
		{
			m_activeButton->onTouchUp( Point(-1,-1) );
			m_activeButton = NULL;
		}

		IWidget* wdgt = m_activePane ? m_activePane->hitTest(pt) : NULL;

		if ( wdgt != NULL ) 
		{
			m_activeButton = wdgt;
			m_activeButton->onTouchDown(pt);
		}
	}

	void onTouchUpdate(int x, int y)
	{
		Point pt(x,y);
	
		if ( m_activeButton )
		{
			m_activeButton->onTouchUpdate(pt);
		}
	}

	void onTouchUp(int x, int y)
	{
		Point pt(x,y);
		
		if ( m_activeButton )
		{
			m_activeButton->onTouchUp(pt);
			m_activeButton = NULL;
		}
	}

	void onIter()
	{
		if ( m_fb->needsUpdate() ) 
		{
			m_fb->nextActiveBuffer();
			m_fb->fill( m_fb->getBGColor() );

			if ( m_activePane != NULL ) 
			{
				m_activePane->draw();
			}

			m_fb->switchToActiveBuf();
			m_fb->setUpdated();
		}

		
	}

	void setActivePane(UIPane* pane)
	{
		if ( m_activePane ) 
		{
			m_activePane->onDeactivated();
		}
		
		if ( pane ) 
		{
			pane->onActivated();
		}

		m_activePane = pane;

		m_fb->invalidate();
	}

	void run()
	{
		// request initial paint of UI
		m_fb->invalidate();
		onIter();
	
		while ( !m_shouldQuit ) 
		{
			fd_set rSet;
			
			FD_ZERO(&rSet);

			int tRead = this->TouchDevice::getReadFD();

			int kRead = this->KeyboardDevice::getReadFD();

			int maxFd = 0;

			if ( tRead != -1 )
			{
				FD_SET(tRead, &rSet);
				if ( tRead > maxFd ) 
					maxFd = tRead;
			}
			else
			{
				break;
			}

			if ( kRead != -1 ) 
			{
				FD_SET(kRead, &rSet);
				if ( kRead > maxFd ) 
					maxFd = kRead;
			}

			timeval tv;

			tv.tv_sec = 10;
			tv.tv_usec = 0;

			int sRet = select( maxFd+1, &rSet, NULL, NULL, &tv);
			
			if ( FD_ISSET(tRead, &rSet ) )
			{
				this->TouchDevice::onFDReadReady();
				m_sleepTimeoutManager.onUserEvent();
			}

			if ( FD_ISSET(kRead, &rSet ) ) 
			{
				this->KeyboardDevice::onFDReadReady();
				m_sleepTimeoutManager.onUserEvent();
			}

			onIter();

			if ( m_sleepTimeoutManager.isTimeToSleep() ) 
			{
				CPM::shutdown();
			}
		}
		
		// Do A final re-paint
		onIter();
	}
};



#endif

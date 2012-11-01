#ifndef __UIMANAGER_HPP__
#define __UIMANAGER_HPP__


class UIManager 
	: public TouchDevice
	, public KeyboardDevice 
{
	UIPane*  m_activePane;
	IWidget* m_activeButton;

	FrameBuffer* m_fb;

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

	void onKeyDown(int code)
	{
		if ( code == KEY_POWER )
		{
//			system("/system/bin/shutdown -h now");
		}
	}
	
	void onKeyUp(int code)
	{
		if ( code == KEY_POWER )
		{
//			system("/system/bin/shutdown -h now");
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

			tv.tv_sec = 30;
			tv.tv_usec = 0;

			int sRet = select( maxFd+1, &rSet, NULL, NULL, &tv);
			
			if ( FD_ISSET(tRead, &rSet ) )
			{
				this->TouchDevice::onFDReadReady();
			}

			if ( FD_ISSET(kRead, &rSet ) ) 
			{
				this->KeyboardDevice::onFDReadReady();
			}

			onIter();
		}
		
		// Do A final re-paint
		onIter();
	}
};



#endif
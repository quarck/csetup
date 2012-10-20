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

#include <list>


#include "UI.hpp"

#include "FrameBuffer.hpp"

#include "TouchDevice.hpp"

const char *touch_device = "/dev/input/event2";
const char *fb_device = "/dev/graphics/fb0";

void die(const char *message)
{
	printf("%s\n", message);
	exit(-1);
}

class UIManager 
	: public TouchDevice
	, public WidgetsCollection 
{
	WidgetsCollection widgets;
	
	IWidget* m_activeButton;

	FrameBuffer* m_fb;

public:
	inline UIManager(const char *touchDev, FrameBuffer* fb)
		: TouchDevice ( touchDev, fb->xres(), fb->yres() )
		, WidgetsCollection ( )
		, m_fb (fb)
		, m_activeButton ( NULL )
	{
	}

	void onTouchDown(int x, int y)
	{
		Point pt(x,y);

		if ( m_activeButton ) 
		{
			m_activeButton->onTouchUp( Point(-1,-1) );
			m_activeButton = NULL;
		}

		IWidget* wdgt = this->hitTest(pt);

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
			m_fb->fill(rgb(127, 127, 127));

			this->draw();

			m_fb->switchToActiveBuf();
			m_fb->setUpdated();
		}
	}
};


class QuitButton : public BasicButton
{
	UIManager *m_manager;
public:
	inline QuitButton(
			UIManager * manager,
			FrameBuffer* fb, 
			const Rect& visual, 
			const Rect& active, 
			const rgb& color, 
			const rgb& activeColor
			)
		: BasicButton(fb, visual, active, color, activeColor)
		, m_manager ( manager )
	{
	}

	void onTouchUp(const Point& pt)
	{
		this->BasicButton::onTouchUp(pt);

		if ( hitTest(pt) ) 
		{
			m_manager->setShouldQuit();
		}
	}
};
	

int main(int argc, char *argv[])
{
	FrameBuffer fb(fb_device);

	if ( !fb.isValid() ) 
		die("Failed to open frame buffer device");

	UIManager manager( touch_device, &fb );

	if ( !manager.isValid() ) 
		die("Failed to open touch screen device");

	
//	fb.nextActiveBuffer();
//	fb.fill(rgb(127, 127, 127));
	
	rgb color (255,255,255);
	rgb colorActive(0,255,255);
	
	for (int ix = 0; ix < 6; ix ++ )
	{
		for (int iy = 0; iy < 10; iy ++ ) 
		{
			Rect visual (ix * 80 + 40, iy * 80 + 40, 60, 60);
			Rect active (ix * 80 + 30, iy * 80 + 30, 80, 80);

			if ( ix == 0 && iy == 0 ) 
			{
				manager.add(  
					new QuitButton(&manager, &fb, visual, active, color, colorActive)
				);
			}
			else
			{
				manager.add(  
					new BasicButton(&fb, visual, active, color, colorActive)
				);
			}
		}
	}

	// request repainting
	fb.invalidate();
	manager.onIter();

	manager.run();

	
	fb.nextActiveBuffer();
	fb.fill(rgb(0, 0, 0));
	fb.switchToActiveBuf();

	return 0;
}


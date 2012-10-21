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

#include "Image.hpp"

#include "BMP.hpp"

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


class QuitButton : public ImageButton
{
	UIManager *m_manager;
public:
	inline QuitButton(
			UIManager * manager,
			FrameBuffer* fb, 
			const Rect& visual, 
			const Rect& active, 
			const rgb& color, 
			const rgb& activeColor,
			const Point& imgBasePoint,
			ImageRscSet* rscSet, 
			int imgResId0,
			int imgResId1
			)
		: ImageButton(fb, visual, active, color, activeColor, imgBasePoint, rscSet, imgResId0, imgResId1 )
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

	Image* letters = bmp::readImage("/system/csetup/letters.bmp");

	ImageRscSet set(letters);

	char chars[] = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890-=!@#$%^&*()_+[{]};:'\"\\|,<.>/?`~";

	for (int i=0; i<sizeof(chars); i++) 
	{
		set.addRes(chars[i], Rect(30*i, 0, 30, 60));
	}

	rgb color (255,255,255);
	rgb colorActive(0,255,255);

	char line0[] = "1234567890";

	char line1lc[] = "qwertyuiop";
	char line1uc[] = "QWERTYUIOP";
	
	char line2lc[] = "asdfghjkl";
	char line2uc[] = "ASDFGHJKL";

	char line3lc[] = "zxcvbnm";
	char line3uc[] = "ZXCVBNM";



	int ybase = 200;
	int xbase = 0;

	for (int i=0; i<sizeof(line0)-1; i++ )
	{
		Rect visual (i * 54 + xbase+2, ybase+10, 50, 60);
		Rect active (i * 54 + xbase, ybase, 54, 80);

		manager.add(  
			new ImageButton( 
				&fb, visual, active, 
				color, colorActive,
				Point(15, 0), 
				&set, line0[i] 
			   )
		);

	}
	
	ybase += 70;

	for (int i=0; i<sizeof(line1lc)-1; i++ )
	{
		Rect visual (i * 54 + xbase+2, ybase+10, 50, 60);
		Rect active (i * 54 + xbase, ybase, 54, 80);

		manager.add(  
			new ImageButton( 
				&fb, visual, active, 
				color, colorActive,
				Point(15, 0), 
				&set, line1lc[i] 
			   )
		);

		manager.add(  
			new ImageButton( 
				&fb, visual, active, 
				color, colorActive,
				Point(15, 0), 
				&set, line1uc[i] 
			   )
		);
	}

	ybase += 70;
	xbase += 54/2;

	for (int i=0; i<sizeof(line2lc)-1; i++ )
	{
		Rect visual (i * 54 + xbase+2, ybase+10, 50, 60);
		Rect active (i * 54 + xbase, ybase, 54, 80);

		manager.add(  
			new ImageButton( 
				&fb, visual, active, 
				color, colorActive,
				Point(15, 0), 
				&set, line2lc[i] 
			   )
		);

		manager.add(  
			new ImageButton( 
				&fb, visual, active, 
				color, colorActive,
				Point(15, 0), 
				&set, line2uc[i] 
			   )
		);
	}

	ybase += 70;
	xbase += 54/2;

	for (int i=0; i<sizeof(line3lc)-1; i++ )
	{
		Rect visual (i * 54 + xbase+2, ybase+10, 50, 60);
		Rect active (i * 54 + xbase, ybase, 54, 80);

		manager.add(  
			new ImageButton( 
				&fb, visual, active, 
				color, colorActive,
				Point(15, 0), 
				&set, line3lc[i] 
			   )
		);

		manager.add(  
			new ImageButton( 
				&fb, visual, active, 
				color, colorActive,
				Point(15, 0), 
				&set, line3uc[i] 
			   )
		);
	}

	manager.add(  
		new QuitButton(
				&manager, 
				&fb, 
				Rect  (40,  40, 60, 60), 
				Rect ( 30,  30, 80, 80),
				color, 
				colorActive,
				Point(15, 0), 
				&set, 
				'q', 'Q'
			)
		);

	/*
	for (int ix = 0; ix < 6; ix ++ )
	{
		for (int iy = 0; iy < 10; iy ++ ) 
		{
			Rect visual (ix * 80 + 40, iy * 80 + 40, 60, 60);
			Rect active (ix * 80 + 30, iy * 80 + 30, 80, 80);

			manager.add(  
				new ImageButton(
					&fb, 
					visual, 
					active, 
					color, 
					colorActive,
					Point(15, 0), 
					&set, 
					'X' 
				   )
			);
		}
	}
*/
	// request repainting
	fb.invalidate();
	manager.onIter();

	manager.run();

	
	fb.nextActiveBuffer();
	fb.fill(rgb(0, 0, 0));
	fb.switchToActiveBuf();

	return 0;
}


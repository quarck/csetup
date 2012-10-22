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
			m_fb->fill( m_fb->getBGColor() );

			this->draw();

			m_fb->switchToActiveBuf();
			m_fb->setUpdated();
		}
	}
};

class LetterButton: public ImageButton
{
	TextEdit * m_edit;
	int m_char;
	int m_charShifted;
public:
	LetterButton(
			TextEdit* edit, 
			FrameBuffer *fb, 
			const Rect& visual, 
			const Rect& active, 
			const Point& imgBasePoint,
			ImageRscSet* rscSet, 
			int chr, int chrShifted	
		)
		: ImageButton(fb, visual, active, imgBasePoint, rscSet, chr, chrShifted)
		, m_edit (edit)
		, m_char (chr)
		, m_charShifted (chrShifted)
	{
	}

	void onTouchUp(const Point& pt)
	{
		this->BasicButton::onTouchUp(pt);

		if ( hitTest(pt) ) 
		{
			m_edit->appendChar(m_char);
		}
	}
};

class BackspaceButton: public ImageButton
{
	TextEdit * m_edit;
public:
	BackspaceButton(
			TextEdit* edit, 
			FrameBuffer *fb, 
			const Rect& visual, 
			const Rect& active, 
			const Point& imgBasePoint,
			ImageRscSet* rscSet, 
			int chr
		)
		: ImageButton(fb, visual, active, imgBasePoint, rscSet, chr)
		, m_edit (edit)
	{
	}

	void onTouchUp(const Point& pt)
	{
		this->BasicButton::onTouchUp(pt);

		if ( hitTest(pt) ) 
		{
			m_edit->backspace();
		}
	}
};


class QuitButton : public ImageButton
{
	UIManager *m_manager;
	TextEdit *m_edit;
public:
	inline QuitButton(
			UIManager * manager,
			TextEdit  * edit,
			FrameBuffer* fb, 
			const Rect& visual, 
			const Rect& active, 
			const Point& imgBasePoint,
			ImageRscSet* rscSet, 
			int imgResId
			)
		: ImageButton(fb, visual, active, imgBasePoint, rscSet, imgResId)
		, m_manager ( manager )
		, m_edit ( edit )
	{
	}

	void onTouchUp(const Point& pt)
	{
		this->BasicButton::onTouchUp(pt);

		if ( hitTest(pt) ) 
		{
			m_edit->hideLastChr();

			if ( m_edit->getString() == "test123")
			{
				m_manager->setShouldQuit();
			}
		}
	}
};

class ClearButton : public ImageButton
{
	UIManager *m_manager;
	TextEdit *m_edit;
public:
	inline ClearButton(
			UIManager * manager,
			TextEdit  * edit,
			FrameBuffer* fb, 
			const Rect& visual, 
			const Rect& active, 
			const Point& imgBasePoint,
			ImageRscSet* rscSet, 
			int imgResId
			)
		: ImageButton(fb, visual, active, imgBasePoint, rscSet, imgResId)
		, m_manager ( manager )
		, m_edit ( edit )
	{
	}

	void onTouchUp(const Point& pt)
	{
		this->BasicButton::onTouchUp(pt);

		if ( hitTest(pt) ) 
		{
			m_edit->setString("");
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
		set.addRes(chars[i], Rect(30*i, 0, 30, 55));
	}


	TextEdit edit( &fb, Rect(5, 10, 540-54-10, 100), Size(30, 60), Point(10,25), &set, true);

	manager.add(&edit);

	ClearButton clearBtn(
			&manager,
			&edit,
			&fb, 
			Rect (540-50-5, 10, 50, 100), 
			Rect (540-54-5, 10, 54, 100),
			Point(10, 25), 
			&set, 
			'X'
		);

	manager.add(&clearBtn);

	fb.setBGColor( rgb(127,127,127) );
	fb.setColor( rgb(255,255,255 ) );

	//char chars[] = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890-=!@#$%^&*()_+[{]};:'\"\\|,<.>/?`~";
	
	char lineA[] = "-=_+[{]};:";
	char lineB[] = "!@#$%^&*()";

	char line0[] = "1234567890";

	char line1lc[] = "qwertyuiop";
	char line1uc[] = "QWERTYUIOP";
	
	char line2lc[] = "asdfghjkl";
	char line2uc[] = "ASDFGHJKL";

	char line3lc[] = "zxcvbnm";
	char line3uc[] = "ZXCVBNM";
	
	char lineZ[] = "'\"\\|,<.>/?`~";

	int ybase = 200;
	int xbase = 0;

	for (int i=0; i<sizeof(lineZ)-1; i++ )
	{
		Rect visual (i * 45 + xbase, ybase, 45-4, 80-4);
		Rect active (i * 45 + xbase - 2, ybase-2, 45, 80);

		manager.add(  
			new LetterButton(
				&edit,
				&fb, visual, active, 
				Point(6, 15), 
				&set, lineZ[i], lineZ[i]
			   )
		);

	}
	
	ybase += 80;
	
	for (int i=0; i<sizeof(lineA)-1; i++ )
	{
		Rect visual (i * 54 + xbase, ybase, 50, 80-4);
		Rect active (i * 54 + xbase - 2, ybase-2, 54, 80);

		manager.add(  
			new LetterButton(
				&edit,
				&fb, visual, active, 
				Point(10, 15), 
				&set, lineA[i], lineA[i]
			   )
		);

	}
	
	ybase += 80;

	
	for (int i=0; i<sizeof(lineB)-1; i++ )
	{
		Rect visual (i * 54 + xbase, ybase, 50, 80-4);
		Rect active (i * 54 + xbase - 2, ybase-2, 54, 80);

		manager.add(  
			new LetterButton(
				&edit,
				&fb, visual, active, 
				Point(10, 15), 
				&set, lineB[i], lineB[i]
			   )
		);

	}
	
	ybase += 100;

	for (int i=0; i<sizeof(line0)-1; i++ )
	{
		Rect visual (i * 54 + xbase, ybase, 50, 80-4);
		Rect active (i * 54 + xbase - 2, ybase-2, 54, 80);

		manager.add(  
			new LetterButton(
				&edit,
				&fb, visual, active, 
				Point(10, 15), 
				&set, line0[i], line0[i]
			   )
		);

	}
	
	ybase += 80;

	for (int i=0; i<sizeof(line1lc)-1; i++ )
	{
		Rect visual (i * 54 + xbase, ybase, 50, 80-4);
		Rect active (i * 54 + xbase - 2, ybase-2, 54, 80);

		manager.add(  
			new LetterButton(
				&edit,
				&fb, visual, active, 
				Point(10, 15), 
				&set, line1lc[i], line1uc[i] 
			   )
		);
	}

	ybase += 80;
	xbase += 54*2/3;

	for (int i=0; i<sizeof(line2lc)-1; i++ )
	{
		Rect visual (i * 54 + xbase, ybase, 50, 80-4);
		Rect active (i * 54 + xbase - 2, ybase-2, 54, 80);

		manager.add(  
			new LetterButton(
				&edit,
				&fb, visual, active, 
				Point(10, 15), 
				&set, line2lc[i], line2uc[i] 
			   )
		);
	}

	ybase += 80;
	xbase += 54*2/3;

	for (int i=0; i<sizeof(line3lc)-1; i++ )
	{
		Rect visual (i * 54 + xbase, ybase, 50, 80-4);
		Rect active (i * 54 + xbase - 2, ybase-2, 54, 80);

		manager.add(  
			new LetterButton(
				&edit,
				&fb, visual, active, 
				Point(10, 15), 
				&set, line3lc[i], line3uc[i]
			   )
		);
	}
	
	manager.add(
			new BackspaceButton(
				&edit,
				&fb, 
				Rect ( 7 * 54 + xbase + 10, ybase, 50, 80-4), 
				Rect ( 7 * 53 + xbase + 10- 2, ybase-2, 54, 80),
				Point(10, 15), 
				&set, 
				'<'
			)
		);


	manager.add(  
		new QuitButton(
				&manager,
				&edit,
				&fb, 
				Rect (40, 860, 60, 60), 
				Rect (30, 850, 80, 80),
				Point(15, 0), 
				&set, 
				'Q'
			)
		);

	// request repainting
	fb.invalidate();
	manager.onIter();

	manager.run();
	
	fb.nextActiveBuffer();
	fb.fill(rgb(0, 0, 0));
	fb.switchToActiveBuf();

	return 0;
}


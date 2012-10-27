#include <cerrno>
#include <cstddef>

#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

#include <sys/wait.h>

#include <sys/mman.h>

#include <linux/fb.h>
#include <unistd.h>

#include <sys/select.h>

#include "input.h"

#include <list>


#include "UI.hpp"

#include "FrameBuffer.hpp"

#include "TouchDevice.hpp"

#include "KeyboardDevice.hpp"

#include "Image.hpp"

#include "BMP.hpp"

const char *touch_device = "/dev/input/event2";
const char *keyboard_device = "/dev/input/event3";
const char *fb_device = "/dev/graphics/fb0";

const char *lettersbmp = "/system/csetup/letters.bmp";

const char *cryptsetup = "/system/csetup/cryptsetup";


void die(const char *message)
{
	printf("%s\n", message);
	exit(-1);
}

enum {
	ID_SHIFT = 1000, 
	ID_SHIFT_ACTIVE = 1001, 
	ID_BACKSPACE = 1002, 
	ID_OK = 1003, 
	ID_CANCEL = 1004,
	ID_INFO = 1005, 
	ID_EMERGENCY = 1006
};

bool luksOpen(const std::string& dev, const std::string& password, const std::string& dmname)
{
	bool ret = false;

	// popen crashes for now reason (on actually a following fprintf), 
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

class UIManager 
	: public TouchDevice
	, public KeyboardDevice 
	, public WidgetsCollection 
{
	WidgetsCollection widgets;
	
	IWidget* m_activeButton;

	FrameBuffer* m_fb;

	bool m_shouldQuit;

	time_t m_timePowerBtnDown;

public:
	inline UIManager(
				const char *touchDev, 
				const char *kbdDev, 
				FrameBuffer* fb
			)
		: TouchDevice ( touchDev, fb->xres(), fb->yres() )
		, KeyboardDevice ( kbdDev )
		, WidgetsCollection ( )
		, m_fb (fb)
		, m_activeButton ( NULL )
		, m_timePowerBtnDown(0)
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

	void run()
	{
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
	}
};

class Keyboard;

class LetterButton: public ImageButton
{
	TextEdit * m_edit;
	int m_char;
	int m_charShifted;

	bool m_shiftActive;

	Keyboard* m_keyboard;

public:
	LetterButton(
			Keyboard* keyboard,
			TextEdit* edit, 
			FrameBuffer *fb, 
			const Rect& visual, 
			const Rect& active, 
			const Point& imgBasePoint,
			ImageRscSet* rscSet, 
			int chr, int chrShifted	
		)
		: ImageButton(fb, visual, active, imgBasePoint, rscSet, chr, chrShifted)
		, m_keyboard (keyboard)
		, m_edit (edit)
		, m_char (chr)
		, m_charShifted (chrShifted)
		, m_shiftActive( false )
	{
	}

	void onTouchUp(const Point& pt);

	void setShift(bool shift)
	{
		m_shiftActive = shift;

		if ( m_shiftActive ) 
			setActiveImage(1);
		else
			setActiveImage(0);
	}

	bool getShift() const
	{
		return m_shiftActive;
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

		if ( weakHitTest(pt) ) 
		{
			m_edit->backspace();
		}
	}
};


class ShiftButton : public LetterButton 
{
	Keyboard* m_keyboard;
public:
	inline ShiftButton(
			TextEdit* edit,
			Keyboard* kbd,
			FrameBuffer* fb, 
			const Rect& visual, 
			const Rect& active, 
			const Point& imgBasePoint,
			ImageRscSet* rscSet, 
			int imgResId, int imgResIdActive
			)
		: LetterButton(kbd, edit, fb, visual, active, imgBasePoint, rscSet, imgResId, imgResIdActive)
		, m_keyboard(kbd)
	{
	}

	void onTouchUp(const Point& pt);
};


class Keyboard
{
	UIManager* m_manager;
	TextEdit* m_edit;
	FrameBuffer* m_fb;

	bool m_shiftActive;

	std::list<LetterButton*> m_buttons;

public:
	inline Keyboard(
		UIManager * manager,
		TextEdit  * edit,
		FrameBuffer* fb, 
		ImageRscSet* set
		)
		: m_manager ( manager ) 
		, m_edit ( edit )
		, m_fb ( fb )
		, m_shiftActive ( false )
	{
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

		int ybase = 145;
		int xbase = 0;

		for (int i=0; i<sizeof(lineZ)-1; i++ )
		{
			Rect visual (i * 45 + xbase, ybase, 45-4, 80-4);
			Rect active (i * 45 + xbase - 2, ybase-2, 45, 80);

			LetterButton* btn = 
				new LetterButton(
					this,
					m_edit,
					m_fb, visual, active, 
					Point(6, 15), 
					set, lineZ[i], lineZ[i]
				   );

			m_manager->add(btn);
			m_buttons.push_back(btn);
		}
		
		ybase += 80;
		
		for (int i=0; i<sizeof(lineA)-1; i++ )
		{
			Rect visual (i * 54 + xbase, ybase, 50, 80-4);
			Rect active (i * 54 + xbase - 2, ybase-2, 54, 80);

			LetterButton* btn = 
				new LetterButton(
					this, 
					m_edit,
					m_fb, visual, active, 
					Point(10, 15), 
					set, lineA[i], lineA[i]
				   );

			m_manager->add(btn);
			m_buttons.push_back(btn);
		}
		
		ybase += 80;

		
		for (int i=0; i<sizeof(lineB)-1; i++ )
		{
			Rect visual (i * 54 + xbase, ybase, 50, 80-4);
			Rect active (i * 54 + xbase - 2, ybase-2, 54, 80);

			LetterButton* btn = 
				new LetterButton(
					this, 
					m_edit,
					m_fb, visual, active, 
					Point(10, 15), 
					set, lineB[i], lineB[i]
				   );

			m_manager->add(btn);
			m_buttons.push_back(btn);

		}
		
		ybase += 110;

		for (int i=0; i<sizeof(line0)-1; i++ )
		{
			Rect visual (i * 54 + xbase, ybase, 50, 80-4);
			Rect active (i * 54 + xbase - 2, ybase-2, 54, 80);

			LetterButton* btn = 
				new LetterButton(
					this, 
					m_edit,
					m_fb, visual, active, 
					Point(10, 15), 
					set, line0[i], line0[i]
				   );

			m_manager->add(btn);
			m_buttons.push_back(btn);
		}
		
		ybase += 90;

		for (int i=0; i<sizeof(line1lc)-1; i++ )
		{
			Rect visual (i * 54 + xbase, ybase, 50, 80-4);
			Rect active (i * 54 + xbase - 2, ybase-2, 54, 80);

			LetterButton* btn = 
				new LetterButton(
					this, 
					m_edit,
					m_fb, visual, active, 
					Point(10, 15), 
					set, line1lc[i], line1uc[i] 
				   );

			m_manager->add(btn);
			m_buttons.push_back(btn);
		}

		ybase += 80;
		xbase += 54*2/3;

		for (int i=0; i<sizeof(line2lc)-1; i++ )
		{
			Rect visual (i * 54 + xbase, ybase, 50, 80-4);
			Rect active (i * 54 + xbase - 2, ybase-2, 54, 80);

			LetterButton* btn = 
				new LetterButton(
					this, 
					m_edit,
					m_fb, visual, active, 
					Point(10, 15), 
					set, line2lc[i], line2uc[i] 
				   );

			m_manager->add(btn);
			m_buttons.push_back(btn);
		}

		ybase += 80;
		xbase += 54*2/3;

		for (int i=0; i<sizeof(line3lc)-1; i++ )
		{
			Rect visual (i * 54 + xbase, ybase, 50, 80-4);
			Rect active (i * 54 + xbase - 2, ybase-2, 54, 80);

			LetterButton* btn = 
				new LetterButton(
					this, 
					m_edit,
					m_fb, visual, active, 
					Point(10, 15), 
					set, line3lc[i], line3uc[i]
				   );

			m_manager->add(btn);
			m_buttons.push_back(btn);
		}
		
		BackspaceButton* backspace = 
			new BackspaceButton(
					m_edit,
					m_fb, 
					Rect ( 7 * 54 + xbase + 5, ybase, 80, 80-4), 
					Rect ( 7 * 54 + xbase + 5- 2, ybase-2, 84, 80),
					Point(15, 7), 
					set, 
					ID_BACKSPACE
				);
		m_manager->add(backspace);

		ShiftButton* shift = 
			new ShiftButton(
				m_edit,
				this,
				m_fb, 
				Rect (  2, ybase, 66, 80-4), 
				Rect ( 2, ybase-2, 66, 80),
				Point(10, 5), 
				set, 
				ID_SHIFT, ID_SHIFT_ACTIVE
			);
	
		m_manager->add(shift);
		m_buttons.push_back(shift);
	}

	void setShift(bool shift) 
	{
		m_shiftActive = shift;

		for (std::list<LetterButton*>::iterator iter = m_buttons.begin(); 
			iter != m_buttons.end();
			++ iter
		    )
		{
			(*iter)->setShift(m_shiftActive);
		}

		m_fb->invalidate();
	}

	void toggleShift()
	{
		setShift( ! m_shiftActive );
	}
};

void ShiftButton::onTouchUp(const Point& pt)
{
	this->BasicButton::onTouchUp(pt);

	if ( weakHitTest(pt) ) 
	{
		m_keyboard->toggleShift();
	}
}

void LetterButton::onTouchUp(const Point& pt)
{
	this->BasicButton::onTouchUp(pt);

	if ( weakHitTest(pt) ) 
	{
		m_edit->appendChar( m_shiftActive ? m_charShifted : m_char);

		if ( m_keyboard ) 
		{
			m_keyboard->setShift(false);
		}
	}
}

class ActionButton : public ImageButton
{
	UIManager *m_manager;
	TextEdit *m_edit;
	int 	m_id;

public:
	inline ActionButton(
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
		, m_id ( imgResId )
	{
	}

	void onTouchUp(const Point& pt)
	{
		this->BasicButton::onTouchUp(pt);

		m_edit->hideLastChr();

		if ( weakHitTest(pt) ) 
		{
			if ( m_id == ID_OK ) 
			{
				const std::string& str = m_edit->getString();

				if ( str.size() > 6 & 
				 	( str.substr(0,6) == "sdboot" || str.substr(0,6) == "bootsd") )
				{
					std::string password = str.substr(6);

					bool data = luksOpen("/dev/block/mmcblk1p6",  password, "data");
					bool cache = luksOpen("/dev/block/mmcblk1p5",  password, "cache");
					bool devlog = luksOpen("/dev/block/mmcblk1p3",  "emerg", "devlog");

					if ( data && cache && devlog ) 
					{
						m_manager->setShouldQuit();
					}
				}
				else if ( str == "cmdadbd" || str == "cmdadb" )
				{
					system("PATH='/system/bin:/system/xbin:' /sbin/adbd");
					m_edit->setString("");
				}
				else if ( str == "cmdadbd2" || str == "cmdadb2" )
				{
					system("/sbin/adbd");
					m_edit->setString("");
				}
				else
				{
					const std::string& password = str;

					bool data = luksOpen("/dev/block/mmcblk0p23",  password, "data");
					bool cache = luksOpen("/dev/block/mmcblk0p24",  password, "cache");
					bool devlog = luksOpen("/dev/block/mmcblk0p28",  password, "devlog");

					bool sd = luksOpen("/dev/block/mmcblk1p7", m_edit->getString(), "sd");
					
					if ( data && cache && devlog ) 
					{
						m_manager->setShouldQuit();
					}
				}
			}
			else if ( m_id == ID_EMERGENCY )
			{
				std::string password = "emerg";

				bool data = luksOpen("/dev/block/mmcblk1p2",  password, "data");
				bool cache = luksOpen("/dev/block/mmcblk1p1",  password, "cache");
				bool devlog = luksOpen("/dev/block/mmcblk1p3",  password, "devlog");

				if ( data && cache && devlog ) 
				{
					m_manager->setShouldQuit();
				}
			}
			else if ( m_id == ID_INFO ) 
			{
				// N.I.
			}
			else if ( m_id == ID_CANCEL ) 
			{
				system("/system/bin/shutdown -h now");
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

		if ( weakHitTest(pt) ) 
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

	UIManager manager( touch_device, keyboard_device, &fb );

	if ( !manager.isValid() ) 
		die("Failed to open touch screen device");

	Image* letters = bmp::readImage(lettersbmp);

	ImageRscSet set(letters);

	char chars[] = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890-=!@#$%^&*()_+[{]};:'\"\\|,<.>/?`~ ";

	for (int i=0; i<sizeof(chars); i++) 
	{
		set.addRes(chars[i], Rect(30*i, 0, 30, 55));
	}

	set.addRes(ID_SHIFT, Rect(2946-5, 0, 41+10, 55));
	set.addRes(ID_SHIFT_ACTIVE, Rect(3007-5, 0, 41+10, 55));
	set.addRes(ID_BACKSPACE, Rect(3098, 0, 65, 55));
	set.addRes(ID_OK, Rect(3171, 0, 82, 60));
	set.addRes(ID_CANCEL, Rect(3260, 0, 205, 60));
	set.addRes(ID_INFO, Rect(3463, 0, 135, 60));
	set.addRes(ID_EMERGENCY, Rect(3602, 0, 318, 60));

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

	fb.setBGColor( rgb(90,90,127) );
	fb.setColor( rgb(255,255,255 ) );

	Keyboard keyboard(
			&manager,
			&edit,
			&fb, 
			&set
		);

	manager.add(  
		new ActionButton(
				&manager,
				&edit,
				&fb, 
				Rect (20, 755, 209, 80), 
				Rect (20, 755, 209, 80),
				Point(2, 11), 
				&set, 
				ID_CANCEL
			)
		);

	manager.add(  
		new ActionButton(
				&manager,
				&edit,
				&fb, 
				Rect (263, 755, 139, 80), 
				Rect (263, 755, 139, 80),
				Point(2, 11), 
				&set, 
				ID_INFO
			)
		);

	manager.add(  
		new ActionButton(
				&manager,
				&edit,
				&fb, 
				Rect (440, 755, 85, 80), 
				Rect (440, 755, 85, 80),
				Point(2, 11), 
				&set, 
				ID_OK
			)
		);

	manager.add(  
		new ActionButton(
				&manager,
				&edit,
				&fb, 
				Rect (100, 850, 322, 80), 
				Rect (100, 850, 322, 80),
				Point(2, 11), 
				&set, 
				ID_EMERGENCY
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


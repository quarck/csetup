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

#include "Keyboard.hpp"

#include "resources.hpp"

const char *touch_device = "/dev/input/event2";
const char *keyboard_device = "/dev/input/event3";
const char *fb_device = "/dev/graphics/fb0";

const char *lettersbmp = "/system/csetup/letters.bmp";

const char *cryptsetup = "/system/csetup/cryptsetup";

const char *usbmsslun0file = "/sys/devices/virtual/android_usb/android0/f_mass_storage/lun0/file";

const char *devint = "/dev/block/mmcblk0";
const char *devsdcard = "/dev/block/mmcblk1";

const char *partmainsystem = "/dev/block/mmcblk0p22";
const char *partmaindata = "/dev/block/mmcblk0p23";
const char *partmaincache = "/dev/block/mmcblk0p24";
const char *partmaindevlog = "/dev/block/mmcblk0p28";
const char *partmainsd = "/dev/block/mmcblk1p7";

const char *partsdbootdata = "/dev/block/mmcblk1p6";
const char *partsdbootcache = "/dev/block/mmcblk1p5";
const char *partsdbootdevlog = "/dev/block/mmcblk1p3"; // using emerg's devlog - use 'emerg' passwd 

const char *partemergdata = "/dev/block/mmcblk1p2";
const char *partemergcache = "/dev/block/mmcblk1p1";
const char *partemergdevlog = "/dev/block/mmcblk1p3";

const char *emergPasswd = "emerg";


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

bool usbMssExport(const char *device)
{
	bool ret = false;

	FILE *l0file = fopen(usbmsslun0file, "w");
	
	if ( l0file )
	{
		fprintf(l0file, "%s\n", device);
		fclose(l0file);
		ret = true;
	}
	
	return ret;
}

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
		m_activePane = pane;
		m_fb->invalidate();
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


class MainPaneActionButton : public ImageButton
{
	UIManager *m_manager;
	TextEdit *m_edit;
	int 	m_id;

public:
	inline MainPaneActionButton(
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

		m_edit->hideLastChar();

		if ( weakHitTest(pt) ) 
		{
			if ( m_id == ID_OK ) 
			{
				const std::string& str = m_edit->getString();

				if ( ( str.size() > 6 )
					&& 
				     ( str.substr(0,6) == "sdboot" ) )
				{
					std::string password = str.substr(6);

					bool data = luksOpen(partsdbootdata,  password, "data");
					bool cache = luksOpen(partsdbootcache,  password, "cache");
					bool devlog = luksOpen(partsdbootdevlog,  emergPasswd, "devlog");

					if ( data && cache && devlog ) 
					{
						m_manager->setShouldQuit();
					}
				}
				else if ( str == "cmdadb" )
				{
					system("PATH='/system/bin:/system/xbin:' /sbin/adbd");
					m_edit->setString("");
				}
				else if ( str == "cmdadbd" )
				{
					system("PATH='/system/bin:/system/xbin:' /sbin/adbd </dev/null >/dev/null 2>/dev/null &");
					m_edit->setString("");
				}
				else if ( str == "cmdusbsd" )
				{
					usbMssExport(devsdcard);
					m_edit->setString("");
				}
				else if ( str == "cmdusbdata" ) 
				{
					usbMssExport(partmaindata);
					m_edit->setString("");
				}
				else if ( str == "cmdusbsystem" ) 
				{
					usbMssExport(partmainsystem);
					m_edit->setString("");
				}
				else if ( str == "cmdusbcache" ) 
				{
					usbMssExport(partmaincache);
					m_edit->setString("");
				}
				else if ( str == "cmdusbdevlog" )
				{
					usbMssExport(partmaindevlog);
					m_edit->setString("");
				}
				else if ( str == "cmdformat" ) 
				{
#warning ("N.I.")
				}
				else if ( str == "cmdformatsdboot" )
				{
#warning ("N.I.")
				}
				else if ( str == "cmdformatemerg" ) 
				{
#warning ("N.I.")
				}
				else
				{
					const std::string& password = str;

					bool data = luksOpen(partmaindata,  password, "data");
					bool cache = luksOpen(partmaincache,  password, "cache");
					bool devlog = luksOpen(partmaindevlog,  password, "devlog");

					bool sd = luksOpen(partmainsd, m_edit->getString(), "sd");
					
					if ( data && cache && devlog ) 
					{
						m_manager->setShouldQuit();
					}
				}
			}
			else if ( m_id == ID_EMERGENCY )
			{
				bool data = luksOpen(partemergdata,  emergPasswd, "data");
				bool cache = luksOpen(partemergcache,  emergPasswd, "cache");
				bool devlog = luksOpen(partemergdevlog,  emergPasswd, "devlog");

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
	{
		fprintf(stderr, "Failed to open frame buffer device");
		return -1;
	}

	fb.setBGColor( rgb(90,90,127) );
	fb.setColor( rgb(255,255,255 ) );

	UIManager manager( touch_device, keyboard_device, &fb );
	

	if ( !manager.isValid() )
	{
		fprintf(stderr, "Failed to open touch screen device");
		return -1;
	}

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

	
	UIPane mainPane;
	
	
	UIPane infoPane;
	UIPane wipePane;
	UIPane stopUSBPane;


	TextEdit edit( &fb, Rect(5, 10, 540-54-10, 100), Size(30, 60), Point(10,25), &set, true);
	mainPane.add(&edit);

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

	mainPane.add(&clearBtn);

	Keyboard keyboard(
			&mainPane,
			&edit,
			&fb, 
			&set,
			145
		);

	mainPane.add(  
		new MainPaneActionButton(
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

	mainPane.add(  
		new MainPaneActionButton(
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

	mainPane.add(  
		new MainPaneActionButton(
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

	mainPane.add(  
		new MainPaneActionButton(
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

	manager.setActivePane( &mainPane );

	// request repainting
	fb.invalidate();
	manager.onIter();

	manager.run();
	
	fb.nextActiveBuffer();
	fb.fill(rgb(0, 0, 0));
	fb.switchToActiveBuf();

	return 0;
}


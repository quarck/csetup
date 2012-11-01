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
const char *letterssmallbmp = "/system/csetup/letterssmall.bmp";

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
const char *partsdbootsd = "/dev/block/mmcblk1p8"; // using emerg's sd - use 'emerg' passwd

const char *partemergdata = "/dev/block/mmcblk1p2";
const char *partemergcache = "/dev/block/mmcblk1p1";
const char *partemergdevlog = "/dev/block/mmcblk1p3";
const char *partemergsd = "/dev/block/mmcblk1p8";

const char *emergPasswd = "emerg";


bool luksOpen(const std::string& dev, const std::string& password, const std::string& dmname)
{
	bool ret = false;

	// popen crashes for no reason (on actually a following fprintf), 
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

bool luksChangeKey(const std::string& dev, const std::string& oldPassword, const std::string& newPassword)
{
	bool ret = false;

	// popen crashes for no reason (on actually a following fprintf), 
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
			"luksChangeKey", 
			dev.c_str(), 
			NULL
			);
		exit(-1);
	}
	else
	{
		write(pipes[1], oldPassword.c_str(), oldPassword.size());
		write(pipes[1], "\n", 1);
		write(pipes[1], newPassword.c_str(), newPassword.size());
		write(pipes[1], "\n", 1);
		write(pipes[1], newPassword.c_str(), newPassword.size());
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

bool luksFormat(const std::string& dev, const char* keySizeBits, const std::string& newPassword)
{
	bool ret = false;

	// popen crashes for no reason (on actually a following fprintf), 
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
			"--cipher",
			"aes",
			"--key-size",
			keySizeBits,
			"luksFormat", 
			dev.c_str(), 
			NULL
			);
		exit(-1);
	}
	else
	{
		write(pipes[1], "YES\n", 4);

		write(pipes[1], newPassword.c_str(), newPassword.size());
		write(pipes[1], "\n", 1);
		write(pipes[1], newPassword.c_str(), newPassword.size());
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

void echo(const char *str, const char *file)
{
	FILE *f = fopen(file, "w");
	if ( f ) 
	{
		fprintf(f, "%s\n", str);
		fclose(f);
	}
}

void tweakCPUandIOSched()
{
	return ;

	echo("384000", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
	echo("1728000", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
	echo("interactive", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
	
	echo("384000", "/sys/devices/system/cpu/cpu1/cpufreq/scaling_min_freq");
	echo("1728000", "/sys/devices/system/cpu/cpu1/cpufreq/scaling_min_freq");
	echo("interactive", "/sys/devices/system/cpu/cpu1/cpufreq/scaling_governor");

	echo("noop", "/sys/devices/platform/msm_sdcc.1/mmc_host/mmc0/mmc0:0001/block/mmcblk0/queue/scheduler");
	echo("noop", "/sys/devices/platform/msm_sdcc.3/mmc_host/mmc1/mmc1:0001/block/mmcblk1/queue/scheduler");
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



class InfoPaneActionButton : public TextEdit 
{
	UIManager* m_manager;
	UIPane* m_mainPane; 
	int m_id;
public:
	inline InfoPaneActionButton(
			UIManager * manager,
			UIPane* mainPane,
			FrameBuffer* fb, 
			Point ptStart, 
			Size ltrSize, 
			Point offs, 
			ImageRscSet* font, 
			const std::string& string,
			int id
			)
		: TextEdit(fb, ptStart, ltrSize, offs, font, string) 
		, m_manager ( manager )
		, m_mainPane ( mainPane )
		, m_id ( id )
	{
		this->setInvertColorOnActivate ( true );
	}

	void onTouchUp(const Point& pt)
	{
		this->TextEdit::onTouchUp(pt);

		if ( m_id == 0 ) 
		{
			gc()->setBGColor( rgb(90,90,127) );
			m_manager->setActivePane( m_mainPane );
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


class PaneWithEditAndKeyboard : public UIPane 
{
	UIManager* m_manager;
	TextEdit* m_edit;
	ImageRscSet *m_set;

	FrameBuffer* m_fb;

protected:
	UIManager* manager() 
	{ 
		return m_manager; 
	}
	
	TextEdit* edit() 
	{
		return m_edit;
	}
	
	ImageRscSet* set() 
	{
		return m_set;
	}

	FrameBuffer* fb() 
	{
		return m_fb;
	}
public:
	PaneWithEditAndKeyboard(
		UIManager * manager,
		FrameBuffer* fb, 
		ImageRscSet* rscSet, 
		int piKeyboardOffset = 145
		) 
		: m_manager ( manager )
		, m_fb ( fb )
		, m_set ( rscSet )
	{
		m_edit = new TextEdit( m_fb, Rect(5, 10, 540-54-10, 100), Size(30, 60), Point(10,25), m_set, true);
		
		this->add(m_edit);

		ClearButton *clearBtn = new ClearButton(
				m_manager,
				m_edit,
				m_fb, 
				Rect (540-50-5, 10, 50, 100), 
				Rect (540-54-5, 10, 54, 100),
				Point(10, 25), 
				m_set, 
				'X'
			);

		this->add(clearBtn);

		Keyboard *keyboard = new Keyboard(
				this,
				m_edit,
				m_fb, 
				m_set,
				piKeyboardOffset
			);
	}
};


class MainPane : public PaneWithEditAndKeyboard 
{
	UIPane* m_infoPane; 
private:

	UIPane* infoPane() 
	{
		return m_infoPane;
	}
	
public:
	MainPane(
			UIManager * manager,
			FrameBuffer* fb, 
			ImageRscSet* rscSet
		) 
		: PaneWithEditAndKeyboard( manager, fb,  rscSet, 145 )		
		, m_infoPane ( NULL )
	{
		this->add(  
			new CancelButton(
					this, 
					Rect (20, 755, 209, 80), 
					Rect (20, 755, 209, 80),
					Point(2, 11)
				)
			);

		this->add(  
			new InfoButton(
					this,
					Rect (263, 755, 139, 80), 
					Rect (263, 755, 139, 80),
					Point(2, 11)
				)
			);

		this->add(  
			new OKButton(
					this, 
					Rect (440, 755, 85, 80), 
					Rect (440, 755, 85, 80),
					Point(2, 11)
				)
			);

		this->add(  
			new EmergencyButton(
					this, 
					Rect (100, 850, 322, 80), 
					Rect (100, 850, 322, 80),
					Point(2, 11)
				)
			);
	}
	
	void setInfoPane(UIPane* pane)
	{
		m_infoPane = pane;
	}
	
	void welcomeMessage( const std::string& msg)
	{
		UIPane* welcomePane = new UIPane();

		welcomePane->add (
				new TextEdit( fb(), Point(5,100), Size(30,60), Point(5,5), set(), msg)
			);

		manager()->setActivePane ( welcomePane );
	}


	
	class OKButton : public ImageButton
	{
		MainPane* m_pane;
	public:
		inline OKButton(
				MainPane* pane,
				const Rect& visual, 
				const Rect& active, 
				const Point& imgBasePoint
				)
			: ImageButton( pane->fb(), visual, active, imgBasePoint, pane->set(), ID_OK)
			, m_pane ( pane )
		{
		}
		
		void onTouchUp(const Point& pt)
		{
			this->BasicButton::onTouchUp(pt);

			m_pane->edit()->hideLastChar();

			if ( weakHitTest(pt) ) 
			{
				const std::string& str = m_pane->edit()->getString();

				if ( ( str.size() > 6 )
					&& 
				     ( str.substr(0,6) == "sdboot" ) )
				{
					std::string password = str.substr(6);

					bool data = luksOpen(partsdbootdata,  password, "data");
					bool cache = luksOpen(partsdbootcache,  password, "cache");
					bool devlog = luksOpen(partsdbootdevlog,  emergPasswd, "devlog");
				
					bool sd = luksOpen(partsdbootsd, emergPasswd, "sd");

					if ( data && cache && devlog ) 
					{
						tweakCPUandIOSched();

						m_pane->welcomeMessage("Booting into SD");

						m_pane->manager()->setShouldQuit();
					}
				}
				else if ( str == "cmdadb" )
				{
					system("PATH='/system/bin:/system/xbin:' /sbin/adbd");
					m_pane->edit()->setString("");
				}
				else if ( str == "cmdadbd" )
				{
					system("PATH='/system/bin:/system/xbin:' /sbin/adbd </dev/null >/dev/null 2>/dev/null &");
					m_pane->edit()->setString("");
				}
				else if ( str == "cmdusbsd" )
				{
					usbMssExport(devsdcard);
					m_pane->edit()->setString("");
				}
				else if ( str == "cmdusbdata" ) 
				{
					usbMssExport(partmaindata);
					m_pane->edit()->setString("");
				}
				else if ( str == "cmdusbsystem" ) 
				{
					usbMssExport(partmainsystem);
					m_pane->edit()->setString("");
				}
				else if ( str == "cmdusbcache" ) 
				{
					usbMssExport(partmaincache);
					m_pane->edit()->setString("");
				}
				else if ( str == "cmdusbdevlog" )
				{
					usbMssExport(partmaindevlog);
					m_pane->edit()->setString("");
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
				else if ( str == "cmdpasswd" ) 
				{
#warning ("N.I.")
				}
				else if ( str == "cmdpasswdsdboot" )
				{
#warning ("N.I.")
				}
				else if ( str == "cmdpasswdemerg" ) 
				{
#warning ("N.I.")
				}
				else
				{
					const std::string& password = str;

					bool data = luksOpen(partmaindata,  password, "data");
					bool cache = luksOpen(partmaincache,  password, "cache");
					bool devlog = luksOpen(partmaindevlog,  password, "devlog");

					bool sd = luksOpen(partmainsd, password, "sd");
					
					if ( data && cache && devlog ) 
					{
						tweakCPUandIOSched();
						
						m_pane->welcomeMessage("Welcome");

						m_pane->manager()->setShouldQuit();
					}
				}
			}
		}
	};

	class InfoButton : public ImageButton
	{
		MainPane* m_pane;
	public:
		inline InfoButton(
				MainPane* pane,
				const Rect& visual, 
				const Rect& active, 
				const Point& imgBasePoint
				)
			: ImageButton( pane->fb(), visual, active, imgBasePoint, pane->set(), ID_INFO)
			, m_pane ( pane )
		{
		}
		
		void onTouchUp(const Point& pt)
		{
			this->BasicButton::onTouchUp(pt);

			m_pane->edit()->hideLastChar();

			if ( weakHitTest(pt) ) 
			{
				gc()->setBGColor( rgb(255,255,255) );
				m_pane->manager()->setActivePane(m_pane->infoPane());			
			}
		}
	};

	class CancelButton : public ImageButton
	{
		MainPane* m_pane;
	public:
		inline CancelButton(
				MainPane* pane,
				const Rect& visual, 
				const Rect& active, 
				const Point& imgBasePoint
				)
			: ImageButton( pane->fb(), visual, active, imgBasePoint, pane->set(), ID_CANCEL)
			, m_pane ( pane )
		{
		}
		
		void onTouchUp(const Point& pt)
		{
			this->BasicButton::onTouchUp(pt);

			m_pane->edit()->hideLastChar();

			if ( weakHitTest(pt) ) 
			{
				system("/system/bin/shutdown -h now");			
			}		
		}
	};
	
	class EmergencyButton : public ImageButton
	{
		MainPane* m_pane;
	public:
		inline EmergencyButton(
				MainPane* pane,
				const Rect& visual, 
				const Rect& active, 
				const Point& imgBasePoint
				)
			: ImageButton( pane->fb(), visual, active, imgBasePoint, pane->set(), ID_EMERGENCY)
			, m_pane ( pane )
		{
		}
		
		void onTouchUp(const Point& pt)
		{
			this->BasicButton::onTouchUp(pt);

			m_pane->edit()->hideLastChar();

			if ( weakHitTest(pt) ) 
			{
				bool data = luksOpen(partemergdata,  emergPasswd, "data");
				bool cache = luksOpen(partemergcache,  emergPasswd, "cache");
				bool devlog = luksOpen(partemergdevlog,  emergPasswd, "devlog");
					
				bool sd = luksOpen(partemergsd, emergPasswd, "sd");

				if ( data && cache && devlog ) 
				{
					m_pane->welcomeMessage("Starting...");

					m_pane->manager()->setShouldQuit();
				}			
			}		
		}
	};
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

	Image* letterssmall = bmp::readImage(letterssmallbmp);

	ImageRscSet set(letters);
	ImageRscSet smallFont(letterssmall);

	char chars[] = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890-=!@#$%^&*()_+[{]};:'\"\\|,<.>/?`~* ";

	for (int i=0; i<sizeof(chars); i++) 
	{
		set.addRes(chars[i], Rect(30*i, 0, 30, 55));

		smallFont.addRes(chars[i], Rect(15*i + 15*2, 0, 15, 30));
	}

	set.addRes(ID_SHIFT, Rect(2946-5, 0, 41+10, 55));
	set.addRes(ID_SHIFT_ACTIVE, Rect(3007-5, 0, 41+10, 55));
	set.addRes(ID_BACKSPACE, Rect(3098, 0, 65, 55));
	set.addRes(ID_OK, Rect(3171, 0, 82, 60));
	set.addRes(ID_CANCEL, Rect(3260, 0, 205, 60));
	set.addRes(ID_INFO, Rect(3463, 0, 135, 60));
	set.addRes(ID_EMERGENCY, Rect(3602, 0, 318, 60));

	
	UIPane infoPane;
	
	MainPane mainPane( &manager, &fb,  &set );
	
	mainPane.setInfoPane(&infoPane);
	

	InfoPaneActionButton backToMain(
			&manager,
			&mainPane,
			&fb, 
			Point(5,15), 
			Size(30,60), 
			Point(5, 5), 
			&set, 
			"<< Back",
			0
			);

	backToMain.setDrawEdges(true);

	infoPane.add(&backToMain);

#include "info.h"

	for (int i=0; i<sizeof(info)/sizeof(info[0]); i++)
	{
		infoPane.add(
			new TextEdit( &fb, Point(5,70*i+100), Size(30,60), Point(5,5), &set, info[i])
		);
	}
	
	manager.setActivePane( &mainPane );

	// request repainting
	fb.invalidate();
	manager.onIter();

	manager.run();

	manager.onIter(); // draw

	return 0;
}




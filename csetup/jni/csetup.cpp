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


#include "config.h"

#include "UI.hpp"

#include "FrameBuffer.hpp"

#include "TouchDevice.hpp"

#include "KeyboardDevice.hpp"

#include "Image.hpp"

#include "BMP.hpp"

#include "Keyboard.hpp"

#include "LUKS.hpp"

#include "UIManager.hpp"

#include "USBMSS.hpp"

#include "CPUTweak.hpp"

#include "resources.hpp"

class InfoPane : public UIPane 
{
	UIPane* m_mainPane; 

protected:	
	UIPane* mainPane() 
	{
		return m_mainPane;
	}
	
public:
	InfoPane(
			UIManager * manager,
			FrameBuffer* fb, 
			ImageRscSet* rscSet, 
			UIPane* mainPane
		) 
		: UIPane ( manager, rscSet, fb )	
		, m_mainPane ( mainPane )
	{

		BackButton *backToMain = new BackButton(
				this, 
				Point(5,15), 
				Size(30,60), 
				Point(5, 5), 
				set(), 
				"<< Back"
			);

		backToMain->setDrawEdges(true);

		this->add(backToMain);

#include "info.h"

		for (int i=0; i<sizeof(info)/sizeof(info[0]); i++)
		{
			this->add(
				new TextEdit( gc(), Point(5,70*i+100), Size(30,60), Point(5,5), set(), info[i])
			);
		}
	}
	
	void onActivated()
	{
		gc()->setBGColor( rgb(255,255,255) );
	}

	class BackButton : public TextEdit 
	{
		InfoPane* m_pane;
	public:
		inline BackButton(
				InfoPane* pane,
				Point ptStart, 
				Size ltrSize, 
				Point offs, 
				ImageRscSet* font, 
				const std::string& string
				)
			: TextEdit(pane->gc(), ptStart, ltrSize, offs, font, string) 
			, m_pane ( pane )
		{
			this->setInvertColorOnActivate ( true );
		}

		void onBtnClick()
		{
			m_pane->manager()->setActivePane( m_pane->mainPane() );
		}
	};
};



class PaneWithEditAndKeyboard : public UIPane 
{
	TextEdit* m_edit;
protected:	
	TextEdit* edit() 
	{
		return m_edit;
	}
public:
	PaneWithEditAndKeyboard(
		UIManager * manager,
		FrameBuffer* fb, 
		ImageRscSet* rscSet, 
		int piKeyboardOffset
		) 
		: UIPane ( manager, rscSet, fb )
	{
		m_edit = new TextEdit( gc(), Rect(5, 10, 540-54-10, 100), Size(30, 60), Point(10,25), set(), true);
		
		this->add(m_edit);

		ClearButton *clearBtn = new ClearButton(
				m_edit,
				gc(), 
				Rect (540-50-5, 10, 50, 100), 
				Rect (540-54-5, 10, 54, 100),
				Point(10, 25), 
				set()
			);

		this->add(clearBtn);

		Keyboard *keyboard = new Keyboard(
				this,
				m_edit,
				gc(), 
				set(),
				piKeyboardOffset + 145
			);
	}
		
	void onActivated()
	{
		gc()->setBGColor( rgb(90,90,127) );
		gc()->setColor( rgb(255,255,255 ) );
	}
		
	class ClearButton : public ImageButton
	{
		TextEdit* m_edit;		
	public:
		inline ClearButton(
				TextEdit  * edit,
				IGraphics* gc, 
				const Rect& visual, 
				const Rect& active, 
				const Point& imgBasePoint,
				ImageRscSet* rscSet
				)
			: ImageButton(gc, visual, active, imgBasePoint, rscSet, 'X')
			, m_edit ( edit )
		{
		}

		void onBtnClick()
		{
			m_edit->setString("");
		}
	};

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
		: PaneWithEditAndKeyboard( manager, fb,  rscSet, 0 )		
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
		UIPane* welcomePane = new UIPane( manager(), set(), gc() );

		welcomePane->add (
				new TextEdit( gc(), Point(5,100), Size(30,60), Point(5,5), set(), msg)
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
			: ImageButton( pane->gc(), visual, active, imgBasePoint, pane->set(), ID_OK)
			, m_pane ( pane )
		{
		}
		
		void onTouchUp(const Point& pt)
		{
			this->BasicButton::onTouchUp(pt);

			m_pane->edit()->hideLastChar();
			
		}
		
		void onBtnClick()
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
			: ImageButton( pane->gc(), visual, active, imgBasePoint, pane->set(), ID_INFO)
			, m_pane ( pane )
		{
		}
		
		void onTouchUp(const Point& pt)
		{
			this->BasicButton::onTouchUp(pt);

			m_pane->edit()->hideLastChar();
		}
		
		void onBtnClick()
		{
			m_pane->manager()->setActivePane(
					m_pane->infoPane()
				);
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
			: ImageButton( pane->gc(), visual, active, imgBasePoint, pane->set(), ID_CANCEL)
			, m_pane ( pane )
		{
		}
		
		void onTouchUp(const Point& pt)
		{
			this->BasicButton::onTouchUp(pt);

			m_pane->edit()->hideLastChar();
		}
		void onBtnClick()
		{
			system("/system/bin/shutdown -h now");			
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
			: ImageButton( pane->gc(), visual, active, imgBasePoint, pane->set(), ID_EMERGENCY)
			, m_pane ( pane )
		{
		}
		
		void onTouchUp(const Point& pt)
		{
			this->BasicButton::onTouchUp(pt);

			m_pane->edit()->hideLastChar();
		}

		void onBtnClick()
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
	};
}; 


int main(int argc, char *argv[])
{
	//
	// open FB dev
	//
	FrameBuffer fb(fb_device);

	if ( !fb.isValid() ) 
	{
		fprintf(stderr, "Failed to open frame buffer device");
		return -1;
	}

	//
	// initalize UI Manager
	//
	UIManager manager( touch_device, keyboard_device, &fb );
	

	if ( !manager.isValid() )
	{
		fprintf(stderr, "Failed to open touch screen device");
		return -1;
	}

	//
	// Load resources 
	//
	Image* letters = bmp::readImage(lettersbmp);

	ImageRscSet set(letters);

	char chars[] = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890-=!@#$%^&*()_+[{]};:'\"\\|,<.>/?`~* ";

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

	//// 
	/// CREATE THE UI
	////
	
	MainPane mainPane( 
		&manager, 
		&fb,  
		&set 
	);
	
	InfoPane infoPane( 
		&manager, 
		&fb,  
		&set, 
		&mainPane 
	);

	mainPane.setInfoPane(&infoPane);

	manager.setActivePane( &mainPane );

	// Enter the main loop
	manager.run();

	return 0;
}




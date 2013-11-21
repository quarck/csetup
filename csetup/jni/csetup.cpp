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

#include "debuglog.h"

#include "input.h"

#include <list>
#include <vector>

#include "config.h"

#include "UI.hpp"

#include "PM.hpp"

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

#include "emergramdisk.hpp"

#include "securityKey.hpp"


class CountdownPane 
	: public UIPane 
	, public CUIPingManager::IUIPingReceiver
{
	class UnlockButton : public TextEdit 
	{
		CountdownPane* m_pane;
	public:
		inline UnlockButton(
				CountdownPane* pane,
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
			m_pane->manager()->setActivePane( m_pane->nextPane() );
		}
	};


	UIPane* m_nextPane;

	int m_cnt;

	UnlockButton* m_button;

protected:	
	UIPane* nextPane() 
	{
		return m_nextPane;
	}

public:
	CountdownPane(
			UIManager * manager,
			IGraphics * gc, 
			ImageRscSet* rscSet, 
			UIPane* nextPane, 
			int initialCount
		) 
		: UIPane ( manager, rscSet, gc )	
		, m_nextPane ( nextPane )
		, m_cnt ( initialCount )
	{
		char buf[128];
		snprintf(buf, sizeof(buf), "%02d", m_cnt);

		m_button = new UnlockButton(
				this, 
				Point(5,15), 
				Size(30,60), 
				Point(5, 5), 
				set(), 
				buf
			);

		this->add(m_button);

		manager->setPingReceiver( this );
		manager->setPingInterval( 1 );
	}

	void onPing()
	{
		m_cnt--;
	
		char buf[128];
		snprintf(buf, sizeof(buf), "%02d", m_cnt);

		m_button->setString(buf);
		gc()->invalidate();

		if ( m_cnt == 0 ) 
		{

			bool data = createramdisk("/data", "/dev/mapper/data", 0) == 0 ? true : false; 
			//bool data = linkOpen(part_emerg, "data");

			if ( data ) 
			{
				manager()->setShouldQuit();
			}
			else
			{
				// failback
				manager()->setActivePane( nextPane() );
			}
		}
	}

	void onActivated()
	{
		gc()->setBGColor( rgb(0,0,0) );
	}

	void onDeactivated()
	{
		manager()->setPingReceiver( NULL );
	}

};


class InfoPane : public UIPane 
{
	UIPane* m_prevPane; 

protected:	
	UIPane* prevPane() 
	{
		return m_prevPane;
	}
	
public:
	InfoPane(
			UIManager * manager,
			IGraphics * gc, 
			ImageRscSet* rscSet, 
			UIPane* prevPane, 
			const char** lines, 
			int numLines, 
			const std::string backCaption = "<< Back"
		) 
		: UIPane ( manager, rscSet, gc )	
		, m_prevPane ( prevPane )
	{

		BackButton *backToMain = new BackButton(
				this, 
				Point(5,15), 
				Size(30,60), 
				Point(5, 5), 
				set(), 
				backCaption
			);

		backToMain->setDrawEdges(true);

		this->add(backToMain);

		for (int i=0; i<numLines; i++)
		{
			this->add(
				new TextEdit( gc, Point(5,70*i+100), Size(30,60), Point(5,5), set(), lines[i])
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
			m_pane->manager()->setActivePane( m_pane->prevPane() );
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
		IGraphics* fb, 
		ImageRscSet* rscSet, 
		int piKeyboardOffset
		) 
		: UIPane ( manager, rscSet, fb )
	{
		m_edit = new TextEdit( gc(), Rect(5, 10, 720-72-10, 100), Size(30, 60), Point(10,25), set(), true);
		
		this->add(m_edit);

		ClearButton *clearBtn = new ClearButton(
				m_edit,
				gc(), 
				Rect (720-72-4-5, 10, 50, 100), 
				Rect (720-72-5, 10, 54, 100),
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
		gc()->setBGColor( rgb(0, 0, 0) );
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

class FormatPane : public PaneWithEditAndKeyboard 
{
	UIPane* m_prevPane; 

	TextEdit * m_label;

	bool m_bKeyOK;
	std::string m_newPassword;
	std::string m_newPasswordConfirmation;

	std::string m_dataDevice;
	std::string m_cacheDevice;
	std::string m_devlogDevice;
	std::string m_sdDevice;

	std::string m_strDataKeySize;

protected:	
	UIPane* prevPane() 
	{
		return m_prevPane;
	}

public:
	void setPassword(const std::string& pass)
	{
		m_newPassword = pass;
		m_newPasswordConfirmation = pass;
	}

	FormatPane(
			UIManager * manager,
			IGraphics* fb, 
			ImageRscSet* rscSet,
			UIPane* prevPane,
			const std::string& dataDevice,
			const std::string& cacheDevice,
			const std::string& devlogDevice,
			const std::string& sdDevice,
			const std::string& strKeySize
		) 
		: PaneWithEditAndKeyboard( manager, fb,  rscSet, 90 )		
		, m_prevPane ( prevPane )
		, m_dataDevice ( dataDevice ) 
		, m_cacheDevice ( cacheDevice ) 
		, m_devlogDevice ( devlogDevice ) 
		, m_sdDevice ( sdDevice )
		, m_strDataKeySize ( strKeySize )
		, m_bKeyOK ( false )
	{

		m_label = new TextEdit( fb, Rect(5, 110, 720-5, 80), Size(30, 60), Point(10,10), set(), false);

		m_label->setString("Security Key");

		this->add(m_label);

		this->add(  
			new CancelButton(
					this, 
					Rect (20, 920+90, 209, 80), 
					Rect (20, 920+90, 209, 80),
					Point(2, 11)
				)
			);

		this->add(  
			new OKButton(
					this, 
					Rect (620, 920+90, 85, 80), 
					Rect (620, 920+90, 85, 80),
					Point(2, 11)
				)
			);
	}
	
	void resultMessage( const std::string& line1, const std::string& line2 = "", const std::string& line3="")
	{
		std::vector<const char*> lines;
		
		lines.push_back(line1.c_str());
		lines.push_back(line2.c_str());
		lines.push_back(line3.c_str());

		resultMessage(lines);
	}
	
	void resultMessage( std::vector<const char*> lines)
	{
		InfoPane* ip = new InfoPane(
			manager(),
			gc(), 
			set(), 
			prevPane(), 
			&lines[0], 
			lines.size(), 
			"OK"
		); 

		manager()->setActivePane ( ip );
	}



	
	class OKButton : public ImageButton
	{
		FormatPane * m_pane;
	public:
		inline OKButton(
				FormatPane * pane,
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

			if ( !m_pane->m_bKeyOK ) 
			{
				if ( str != securityKey ) 
				{
					m_pane->resultMessage("Invalid");
					return;
				}
				else
				{
					m_pane->m_label->setString("New Password");
					m_pane->edit()->setString("");
					gc()->invalidate();
				}

				m_pane->m_bKeyOK = true;
			}
			else if ( m_pane->m_newPassword == "" ) 
			{
				m_pane->m_newPassword = str;
				m_pane->m_label->setString("Confirm Passwd");
				m_pane->edit()->setString("");
				gc()->invalidate();
			}
			else if ( m_pane->m_newPasswordConfirmation == "" ) 
			{
				m_pane->m_newPasswordConfirmation = str;
			}

			if ( m_pane->m_bKeyOK && m_pane->m_newPassword != "" && m_pane->m_newPasswordConfirmation != "" )
			{
				if ( m_pane->m_newPassword != m_pane->m_newPasswordConfirmation ) 
				{
					m_pane->resultMessage("Passphrases", "do not match.");
					return;
				}
				
				std::vector<const char*> report;

				if ( volFormat( m_pane->m_dataDevice, m_pane->m_strDataKeySize.c_str(), m_pane->m_newPassword , "fmt-data" ) ) 
				{
					report.push_back("Data: OK");
				}
				else
				{
					report.push_back("Data: FAIL!");
				}
				
				if ( m_pane->m_sdDevice != "" ) 
				{
					if ( volFormat( m_pane->m_sdDevice, m_pane->m_strDataKeySize.c_str(), m_pane->m_newPassword, "fmt-sd" ) ) 
					{
						report.push_back("SD: OK");
					}
					else
					{
						report.push_back("SD: FAIL!");
					}
				}


				if ( m_pane->m_cacheDevice != "" ) 
				{
					if ( volFormat( m_pane->m_cacheDevice, "128", m_pane->m_newPassword, "fmt-cache") ) 
					{
						report.push_back("Cache: OK");
					}
					else
					{
						report.push_back("Cache: FAIL!");
					}
				}

				if ( m_pane->m_devlogDevice != "" ) 
				{
					if ( volFormat( m_pane->m_devlogDevice, "128", m_pane->m_newPassword, "fmt-devlog") ) 
					{
						report.push_back("Devlog: OK");
					}
					else
					{
						report.push_back("Devlog: FAIL!");
					}
				}

				m_pane->resultMessage ( report );
			}
		}
	};

	class CancelButton : public ImageButton
	{
		FormatPane* m_pane;
	public:
		inline CancelButton(
				FormatPane* pane,
				const Rect& visual, 
				const Rect& active, 
				const Point& imgBasePoint
				)
			: ImageButton( pane->gc(), visual, active, imgBasePoint, pane->set(), ID_CANCEL)
			, m_pane ( pane )
		{
		}
		
		void onBtnClick()
		{
			m_pane->manager()->setActivePane( 
				m_pane->prevPane() 
			);
		}		
	};
}; 



class PasswordChangePane : public PaneWithEditAndKeyboard 
{
	UIPane* m_prevPane; 

	TextEdit * m_label;

	std::string m_oldPassword;
	std::string m_newPassword;
	std::string m_newPasswordConfirmation;

	std::string m_dataDevice;
	std::string m_cacheDevice;
	std::string m_devlogDevice;
	std::string m_sdDevice;

protected:	
	UIPane* prevPane() 
	{
		return m_prevPane;
	}

public:
	PasswordChangePane(
			UIManager * manager,
			IGraphics* fb, 
			ImageRscSet* rscSet,
			UIPane* prevPane,
			const std::string& dataDevice,
			const std::string& cacheDevice,
			const std::string& devlogDevice,
			const std::string& sdDevice
		) 
		: PaneWithEditAndKeyboard( manager, fb,  rscSet, 90 )		
		, m_prevPane ( prevPane )
		, m_dataDevice ( dataDevice ) 
		, m_cacheDevice ( cacheDevice ) 
		, m_devlogDevice ( devlogDevice ) 
		, m_sdDevice ( sdDevice )
	{

		m_label = new TextEdit( fb, Rect(5, 110,720-5, 80), Size(30, 60), Point(10,10), set(), false);

		m_label->setString("Current Pwd");

		this->add(m_label);

		this->add(  
			new CancelButton(
					this, 
					Rect (20, 920+90, 209, 80), 
					Rect (20, 920+90, 209, 80),
					Point(2, 11)
				)
			);

		this->add(  
			new OKButton(
					this, 
					Rect (620, 920+90, 85, 80), 
					Rect (620, 920+90, 85, 80),
					Point(2, 11)
				)
			);
	}
	
	void resultMessage( const std::string& line1, const std::string& line2 = "", const std::string& line3="")
	{
		std::vector<const char*> lines;
		
		lines.push_back(line1.c_str());
		lines.push_back(line2.c_str());
		lines.push_back(line3.c_str());

		resultMessage(lines);
	}
	
	void resultMessage( std::vector<const char*> lines)
	{
		InfoPane* ip = new InfoPane(
			manager(),
			gc(), 
			set(), 
			prevPane(), 
			&lines[0], 
			lines.size(), 
			"OK"
		); 

		manager()->setActivePane ( ip );
	}

	void onActivated()
	{
		this->PaneWithEditAndKeyboard::onActivated();
	}	

	
	class OKButton : public ImageButton
	{
		PasswordChangePane* m_pane;
	public:
		inline OKButton(
				PasswordChangePane* pane,
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

			if ( m_pane->m_oldPassword == "" ) 
			{
				m_pane->m_oldPassword = str;
				m_pane->m_label->setString("New Password");
				m_pane->edit()->setString("");
				gc()->invalidate();
			}
			else if ( m_pane->m_newPassword == "" ) 
			{
				m_pane->m_newPassword = str;
				m_pane->m_label->setString("Confirm Passwd");
				m_pane->edit()->setString("");
				gc()->invalidate();
			}
			else if ( m_pane->m_newPasswordConfirmation == "" ) 
			{
				m_pane->m_newPasswordConfirmation = str;

				if ( m_pane->m_newPassword != m_pane->m_newPasswordConfirmation ) 
				{
					m_pane->resultMessage("Passphrases", "do not match.");
					return;
				}

				if ( luksChangeKey ( m_pane->m_dataDevice, m_pane->m_oldPassword, m_pane->m_newPassword ) ) 
				{
					std::vector<const char*> report;

					report.push_back("Data: OK");

					if ( m_pane->m_sdDevice != "" ) 
					{
						if ( luksChangeKey ( m_pane->m_sdDevice, m_pane->m_oldPassword, m_pane->m_newPassword ) ) 
						{
							report.push_back("SD: OK");
						}
						else
						{
							report.push_back("SD: FAIL!");
						}
					}


					if ( m_pane->m_cacheDevice != "" ) 
					{
						if ( luksChangeKey ( m_pane->m_cacheDevice, m_pane->m_oldPassword, m_pane->m_newPassword ) )
						{
							report.push_back("Cache: OK");
						}
						else 
						{
							report.push_back("Cache: FAIL!");
							// it always assumed that cache and devlog are always safe to 
							// re-format - they should not contain any kind of user 
							// or system data that can not be restored
							if ( volFormat( m_pane->m_cacheDevice, "128", m_pane->m_newPassword, "fmt-cache") ) 
							{
								report.push_back("Cache: FMTD");
							}
							else
							{
								report.push_back("Cache: FMT FAIL!");
							}
						}
					}

					if ( m_pane->m_devlogDevice != "" ) 
					{
						if ( luksChangeKey ( m_pane->m_devlogDevice, m_pane->m_oldPassword, m_pane->m_newPassword ) )
						{
							report.push_back("Devlog: OK");
						}
						else 
						{
							report.push_back("Devlog: FAILED!");
							// it always assumed that cache and devlog are always safe to 
							// re-format - they should not contain any kind of user 
							// or system data that can not be restored
							if ( volFormat( m_pane->m_devlogDevice, "128", m_pane->m_newPassword, "fmt-devlog") ) 
							{
								report.push_back("Devlog: FMTD");
							}
							else
							{
								report.push_back("Devlog: FMT FAIL!");
							}
						}
					}

					m_pane->resultMessage ( report );
				}
				else
				{
					m_pane->resultMessage("Data: FAIL!");
				}
			}
		}
	};

	class CancelButton : public ImageButton
	{
		PasswordChangePane* m_pane;
	public:
		inline CancelButton(
				PasswordChangePane* pane,
				const Rect& visual, 
				const Rect& active, 
				const Point& imgBasePoint
				)
			: ImageButton( pane->gc(), visual, active, imgBasePoint, pane->set(), ID_CANCEL)
			, m_pane ( pane )
		{
		}
		
		void onBtnClick()
		{
			m_pane->manager()->setActivePane( 
				m_pane->prevPane() 
			);
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
					Rect (20, 920, 209, 80), 
					Rect (20, 920, 209, 80),
					Point(2, 11)
				)
			);

		this->add(  
			new InfoButton(
					this,
					Rect (293, 920, 139, 80), 
					Rect (293, 920, 139, 80),
					Point(2, 11)
				)
			);

		this->add(  
			new OKButton(
					this, 
					Rect (620, 920, 85, 80), 
					Rect (620, 920, 85, 80),
					Point(2, 11)
				)
			);

		this->add(  
			new EmergencyButton(
					this, 
					Rect (100, 1070, 322, 80), 
					Rect (100, 1070, 322, 80),
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

	void onActivated()
	{
		this->PaneWithEditAndKeyboard::onActivated();
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

			if ( str == "exit" ) 
			{
				exit(0);
			}
			else if ( str == "cmdadb" )
			{
				system("PATH='/system/bin:/system/xbin:/system/csetup:' /sbin/adbd");
				m_pane->edit()->setString("");
				m_pane->manager()->setSleepTimeout(3600*25*365);
			}
			else if ( str == "cmdadbd" )
			{
				system("PATH='/system/bin:/system/xbin:/system/csetup:' /sbin/adbd </dev/null >/dev/null 2>/dev/null &");
				m_pane->edit()->setString("");
				m_pane->manager()->setSleepTimeout(3600*25*365);
			}
			else if ( str == "cmdusbsd" || str == "cmdusb" )
			{
				// whole SD
				usbMssExport(dev_external);
				m_pane->edit()->setString("");
				m_pane->manager()->setSleepTimeout(3600*25*365);
			}
			else if ( str == "cmdusbint" ) 
			{
				usbMssExport(part_internal);
				m_pane->edit()->setString("");
				m_pane->manager()->setSleepTimeout(3600*25*365);
			}
			else if ( str == "cmdusbext" ) 
			{
				usbMssExport(dev_external);
				m_pane->edit()->setString("");
				m_pane->manager()->setSleepTimeout(3600*25*365);
			}
			else if ( str == "cmdformat" ) 
			{
				m_pane->edit()->setString("");

				FormatPane *pane = new FormatPane(
					m_pane->manager(), 
					m_pane->gc(), 
					m_pane->set(), 
					m_pane,
					part_internal,
					"",
					"",
					"",
					"128"	
					);
				
				m_pane->manager()->setSleepTimeout(2000);

				m_pane->manager()->setActivePane ( pane );
			}		
			else if ( str == "cmdpasswd" ) 
			{
				m_pane->edit()->setString("");

				PasswordChangePane *passwd = new PasswordChangePane(
						m_pane->manager(),
						m_pane->gc(),
						m_pane->set(),
						m_pane,
						part_internal,
						"",
						"",
						""	
					);
				
				m_pane->manager()->setSleepTimeout(2000);
				
				m_pane->manager()->setActivePane ( passwd );
			}
			else if ( str == "cmdformatsdboot" ) 
			{
				m_pane->edit()->setString("");

				FormatPane *pane = new FormatPane(
					m_pane->manager(), 
					m_pane->gc(), 
					m_pane->set(), 
					m_pane,
					part_sdboot,
					"",
					"",
					"",
					"128"	
					);
				
				m_pane->manager()->setSleepTimeout(2000);

				m_pane->manager()->setActivePane ( pane );
			}		
			else if ( str == "cmdpasswdsdboot" ) 
			{
				m_pane->edit()->setString("");

				PasswordChangePane *passwd = new PasswordChangePane(
						m_pane->manager(),
						m_pane->gc(),
						m_pane->set(),
						m_pane,
						part_sdboot,
						"",
						"",
						""	
					);
				
				m_pane->manager()->setSleepTimeout(2000);
				
				m_pane->manager()->setActivePane ( passwd );

			}			
			else if ( str == "cmdfailback")
			{
				execl( 	csetupfailback, 
					csetupfailback, 
					NULL
					);
			}
			else if ( str.length() > 6 && str.substr(0, 6) == "sdboot" )
			{
				std::string password = str.substr(6);

				if ( luksOpen(part_sdboot,  password, "data") ) 
				{
					tweakCPUandIOSched();
					
					m_pane->welcomeMessage("Welcome to SD-Boot");

					m_pane->manager()->setShouldQuit();
				}				
			}
			else
			{
				const std::string& password = str;

				if ( luksOpen(part_internal,  password, "data") ) 
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
			CPM::shutdown();
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
			bool data = createramdisk("/data", "/dev/mapper/data", 0) == 0 ? true : false; 
			//bool data = linkOpen(part_emerg, "data");

			if ( data ) 
			{
				m_pane->welcomeMessage("Starting...");

				m_pane->manager()->setShouldQuit();
			}			
		}		
	};
}; 


int main(int argc, char *argv[])
{
	if (argc == 2 && strcmp(argv[1], "--ensure-data-media") == 0 ) 
	{
		mkdir("/data/media", 0770);
		mkdir("/cache/dalvik-cache", 775);
		chown("/cache/dalvik-cache", 1000, 1000);
		mount("/cache/dalvik-cache", "/data/dalvik-cache", "none", MS_BIND, "");
		return 0;
	}
	
	CPUStartupSetup();

	int maxBuffers = 2;
	
	if ( argc >= 2 && strcmp(argv[1], "-u") == 0)
		maxBuffers = 10;

	//
	// open FB dev
	//
	FrameBuffer fb(fb_device, maxBuffers);

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

	CountdownPane countdownPane(
			&manager,
			&fb, 
			&set, 
			&mainPane, 
			6
		); 


#include "info.h"

	InfoPane infoPane( 
		&manager, 
		&fb,  
		&set, 
		&mainPane, 
		info, 
		sizeof(info)/sizeof(info[0])
	);

	mainPane.setInfoPane(&infoPane);

	manager.setActivePane( &countdownPane );
	//manager.setActivePane( &mainPane );

	// Enter the main loop
	manager.run();

	return 0;
}




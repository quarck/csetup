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

#ifndef __KEYBOARD_HPP__
#define __KEYBOARD_HPP__

#include <list>

#include "UI.hpp"

#include "Image.hpp"

#include "IKeyboardEditable.hpp"

#include "resources.hpp"

class Keyboard;

class LetterButton: public ImageButton
{
	IKeyboardEditable* m_editable;
	int m_char;
	int m_charShifted;

	bool m_shiftActive;

	Keyboard* m_keyboard;

public:
	LetterButton(
			Keyboard* keyboard,
			IKeyboardEditable* edit, 
			IGraphics* gc, 
			const Rect& visual, 
			const Rect& active, 
			const Point& imgBasePoint,
			ImageRscSet* rscSet, 
			int chr, int chrShifted	
		)
		: ImageButton(gc, visual, active, imgBasePoint, rscSet, chr, chrShifted)
		, m_keyboard (keyboard)
		, m_editable (edit)
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
	IKeyboardEditable* m_editable;
public:
	BackspaceButton(
			IKeyboardEditable* edit, 
			IGraphics* gc, 
			const Rect& visual, 
			const Rect& active, 
			const Point& imgBasePoint,
			ImageRscSet* rscSet, 
			int chr
		)
		: ImageButton(gc, visual, active, imgBasePoint, rscSet, chr)
		, m_editable (edit)
	{
	}

	void onTouchUp(const Point& pt)
	{
		this->BasicButton::onTouchUp(pt);

		if ( weakHitTest(pt) ) 
		{
			m_editable->backspace();
		}
	}
};


class ShiftButton : public LetterButton 
{
	Keyboard* m_keyboard;
public:
	inline ShiftButton(
			IKeyboardEditable* edit,
			Keyboard* kbd,
			IGraphics* gc, 
			const Rect& visual, 
			const Rect& active, 
			const Point& imgBasePoint,
			ImageRscSet* rscSet, 
			int imgResId, int imgResIdActive
			)
		: LetterButton(kbd, edit, gc, visual, active, imgBasePoint, rscSet, imgResId, imgResIdActive)
		, m_keyboard(kbd)
	{
	}

	void onTouchUp(const Point& pt);
};


class Keyboard
{
	UIPane* m_uiPane;
	IKeyboardEditable* m_editable;
	IGraphics* m_gc;

	bool m_shiftActive;

	int m_offsetY;

	std::list<LetterButton*> m_buttons;

public:
	inline Keyboard(
		UIPane * uiPane,
		IKeyboardEditable  * edit,
		IGraphics* gc, 
		ImageRscSet* set,
		int offsetY 
		)
		: m_uiPane ( uiPane ) 
		, m_editable ( edit )
		, m_gc ( gc )
		, m_shiftActive ( false )
		, m_offsetY ( offsetY )
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

		int ybase = offsetY;
		int xbase = 0;

		for (int i=0; i<sizeof(lineZ)-1; i++ )
		{
			Rect visual (i * 60 + xbase, ybase, 60-4, 100-4);
			Rect active (i * 60 + xbase - 2, ybase-2, 60, 100);

			LetterButton* btn = 
				new LetterButton(
					this,
					m_editable,
					m_gc, visual, active, 
					Point(6, 15), 
					set, lineZ[i], lineZ[i]
				   );

			m_uiPane->add(btn);
			m_buttons.push_back(btn);
		}
		
		ybase += 100;
		
		for (int i=0; i<sizeof(lineA)-1; i++ )
		{
			Rect visual (i * 72 + xbase, ybase, 72-4, 100-4);
			Rect active (i * 72 + xbase - 2, ybase-2, 72, 100);

			LetterButton* btn = 
				new LetterButton(
					this, 
					m_editable,
					m_gc, visual, active, 
					Point(10, 15), 
					set, lineA[i], lineA[i]
				   );

			m_uiPane->add(btn);
			m_buttons.push_back(btn);
		}
		
		ybase += 100;

		
		for (int i=0; i<sizeof(lineB)-1; i++ )
		{
			Rect visual (i * 72 + xbase, ybase, 72-4, 100-4);
			Rect active (i * 72 + xbase - 2, ybase-2, 72, 100);

			LetterButton* btn = 
				new LetterButton(
					this, 
					m_editable,
					m_gc, visual, active, 
					Point(10, 15), 
					set, lineB[i], lineB[i]
				   );

			m_uiPane->add(btn);
			m_buttons.push_back(btn);

		}
		
		ybase += 130;

		for (int i=0; i<sizeof(line0)-1; i++ )
		{
			Rect visual (i * 72 + xbase, ybase, 72-4, 100-4);
			Rect active (i * 72 + xbase - 2, ybase-2, 72, 100);

			LetterButton* btn = 
				new LetterButton(
					this, 
					m_editable,
					m_gc, visual, active, 
					Point(10, 15), 
					set, line0[i], line0[i]
				   );

			m_uiPane->add(btn);
			m_buttons.push_back(btn);
		}
		
		ybase += 110;

		for (int i=0; i<sizeof(line1lc)-1; i++ )
		{
			Rect visual (i * 72 + xbase, ybase, 72-4, 100-4);
			Rect active (i * 72 + xbase - 2, ybase-2, 72, 100);

			LetterButton* btn = 
				new LetterButton(
					this, 
					m_editable,
					m_gc, visual, active, 
					Point(10, 15), 
					set, line1lc[i], line1uc[i] 
				   );

			m_uiPane->add(btn);
			m_buttons.push_back(btn);
		}

		ybase += 100;
		xbase += 72*2/3;

		for (int i=0; i<sizeof(line2lc)-1; i++ )
		{
			Rect visual (i * 72 + xbase, ybase, 72-4, 100-4);
			Rect active (i * 72 + xbase - 2, ybase-2, 72, 100);

			LetterButton* btn = 
				new LetterButton(
					this, 
					m_editable,
					m_gc, visual, active, 
					Point(10, 15), 
					set, line2lc[i], line2uc[i] 
				   );

			m_uiPane->add(btn);
			m_buttons.push_back(btn);
		}

		ybase += 100;
		xbase += 72*2/3;

		for (int i=0; i<sizeof(line3lc)-1; i++ )
		{
			Rect visual (i * 72 + xbase, ybase, 72-4, 100-4);
			Rect active (i * 72 + xbase - 2, ybase-2, 72, 100);

			LetterButton* btn = 
				new LetterButton(
					this, 
					m_editable,
					m_gc, visual, active, 
					Point(10, 15), 
					set, line3lc[i], line3uc[i]
				   );

			m_uiPane->add(btn);
			m_buttons.push_back(btn);
		}
		
		BackspaceButton* backspace = 
			new BackspaceButton(
					m_editable,
					m_gc, 
					Rect ( 7 * 72 + xbase + 5, ybase, 100, 100-4), 
					Rect ( 7 * 72 + xbase + 5- 2, ybase-2, 104, 100),
					Point(15, 7), 
					set, 
					ID_BACKSPACE
				);
		m_uiPane->add(backspace);

		ShiftButton* shift = 
			new ShiftButton(
				m_editable,
				this,
				m_gc, 
				Rect (  2, ybase, 88, 100-4), 
				Rect ( 2, ybase-2, 88, 100),
				Point(10, 5), 
				set, 
				ID_SHIFT, ID_SHIFT_ACTIVE
			);
	
		m_uiPane->add(shift);
		m_buttons.push_back(shift);
	}

	inline ~Keyboard()
	{
		for (std::list<LetterButton*>::iterator iter = m_buttons.begin(); 
			iter != m_buttons.end();
			++ iter
		    )
		{
			m_uiPane->remove( *iter );
			delete *iter;
		}

		m_buttons.clear();
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

		m_gc->invalidate();
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
		m_editable->appendChar( m_shiftActive ? m_charShifted : m_char);

		if ( m_keyboard ) 
		{
			m_keyboard->setShift(false);
		}
	}
}

#endif

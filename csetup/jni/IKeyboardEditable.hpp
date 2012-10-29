#ifndef __IKEYBOARD_EDITABLE_HPP__
#define __IKEYBOARD_EDITABLE_HPP__ 

class IKeyboardEditable
{
public:
	virtual void appendChar(char c) = 0;

	virtual void backspace() = 0;

	virtual void hideLastChar() = 0;
};

#endif

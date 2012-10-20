#ifndef __UI_HPP__
#define __UI_HPP__

#include <sys/types.h>

#include <list>
#include <algorithm>


struct rgb
{
	u_int32_t m_rgb;

	inline rgb(unsigned int r, unsigned int g, unsigned int b)
	{
		m_rgb = (r << 16) | (g << 8) | (b);
	}

	operator u_int32_t () const 
	{
		return m_rgb;
	}

	unsigned char r() const 
	{
		return (m_rgb & 0x00ff0000 ) >> 16;
	}

	unsigned char g() const 
	{
		return (m_rgb & 0x0000ff00 ) >> 8;
	}

	unsigned char b() const 
	{
		return (m_rgb & 0x000000ff ) >> 0;
	}
};


class Point
{
	int m_x;
	int m_y;
public:
	inline int getX() const 
	{
		return m_x; 
	}

	inline void setX(int v) 
	{ 
		m_x = v; 
	}
	
	inline int getY() const 
	{
		return m_y; 
	}

	inline void setY(int v) 
	{ 
		m_y = v; 
	}
	
	inline Point(int x, int y) 
		: m_x(x)
		, m_y(y) 
	{
	}
};

class Size
{
	int m_width;
	int m_height;

public:
	inline int getWidth() const 
	{
		return m_width; 
	}

	inline void setWidth(int v) 
	{ 
		m_width = v; 
	}
	
	inline int getHeight() const 
	{
		return m_height; 
	}

	inline void setHeight(int v) 
	{ 
		m_height = v; 
	}
	
	inline Size(int w, int h) 
		: m_width(w)
		, m_height(h) 
	{
	}
};

class Rect
{
	Point origin;
	Size size;
	
public:
	inline const Point& getOrigin() const 
	{
		return origin; 
	}

	inline void setOrigin(Point& v) 
	{
		origin = v; 
	}

	inline const Size& getSize() const 
	{
		return size; 
	}
	
	inline void setSize(Size& v) 
	{ 
		size = v; 
	}
	
	inline Rect(int x, int y, int w, int h)  
		: origin(x,y)
		, size(w,h) 
	{ 
	}
public:
	inline bool inside(const Point& pt) 
	{
		if ( ( pt.getX() >= origin.getX() && pt.getX() < origin.getX() + size.getWidth() ) 
			&& 
		     ( pt.getY() >= origin.getY() && pt.getY() < origin.getY() + size.getHeight() ) 
		    )
		{
			return true;
		}

		return false;
	}
};


class IWidget
{
public:
	virtual bool hitTest(const Point& pt) = 0;

	//
	// Activates when touch is just happened 
	// in the area of this widget. 
	// 
	virtual void onTouchDown(const Point& pt) = 0;

	// Touch up. Widget should check that PT is still 
	// inside the area, and if not -- do not proceed any further events
	virtual void onTouchUp(const Point& pt) = 0;
	
	// update on touch coord
	virtual void onTouchUpdate(const Point& pt) = 0;

	// request to re-draw the widget
	virtual void draw();
};

class WidgetsCollection
{
	std::list<IWidget*> m_listWidgets;

public:
	inline WidgetsCollection()
	{
	}

	inline ~WidgetsCollection()
	{
	}

	inline void add(IWidget* wdgt)
	{
		m_listWidgets.push_back(wdgt);
	}

	inline void remove(IWidget* wdgt)
	{
		std::list<IWidget*>::iterator pos;

		pos = std::find(m_listWidgets.begin(), m_listWidgets.end(), wdgt);

		if ( pos != m_listWidgets.end() )
		{
			m_listWidgets.erase(pos);
		}
	}

	IWidget* hitTest(const Point& pt) const
	{
		IWidget* ret = NULL;

		for (  std::list<IWidget*>::const_iterator iter = m_listWidgets.begin();
			iter != m_listWidgets.end();
			++iter)
		{
			if ( (*iter)->hitTest(pt))
			{
				ret = *iter;
				break;
			}
		}

		return ret;
	}

	void draw() const
	{
		for (  std::list<IWidget*>::const_iterator iter = m_listWidgets.begin();
			iter != m_listWidgets.end();
			++ iter
		    )
		{
			(*iter)->draw();
		}
	}
};


class IGraphics
{
public:
	virtual void fill(const rgb& p) = 0;
	
	virtual void fill(const Rect& area, const rgb& p) = 0;

	virtual void invalidate() = 0;
};


class BasicButton : public IWidget
{
	Rect m_visualRect;
	Rect m_activeRect;

	rgb m_color;
	rgb m_activeColor;
	
	IGraphics* m_gc;

	bool m_active;

public:
	inline BasicButton(
			IGraphics* gc, 
			const Rect& visual, 
			const Rect& active, 
			const rgb& color, 
			const rgb& activeColor
			)
		: m_gc(gc)
		, m_visualRect(visual)
		, m_activeRect(active)
		, m_color(color)
		, m_activeColor(activeColor)
	{
	}
	
	inline void draw( )
	{
		if ( m_active ) 
		{
			m_gc->fill(m_visualRect, m_activeColor);
		}
		else
		{
			m_gc->fill(m_visualRect, m_color);
		}
	}
	
	bool hitTest(const Point& pt)
	{
		return m_activeRect.inside(pt);
	}
	
	void onTouchDown(const Point& pt)
	{
		m_gc->invalidate();
		m_active = true;
	}
	
	void onTouchUp(const Point& pt)
	{
		m_gc->invalidate();
		m_active = false;
	}
	
	// update on touch coord
	void onTouchUpdate(const Point& pt)
	{
	}
};



#endif

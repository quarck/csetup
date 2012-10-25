#ifndef __UI_HPP__
#define __UI_HPP__

#include <sys/types.h>

#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <string>

#include "Image.hpp"

struct rgb
{
	u_int32_t m_rgb;

	inline rgb(unsigned int r, unsigned int g, unsigned int b)
	{
		m_rgb = (b << 16) | (g << 8) | (r);
	}

	rgb negative()
	{
		rgb ret (0,0,0);
		ret.m_rgb = (~m_rgb) & 0x00ffffff;
		return ret;
	}

	operator u_int32_t () const 
	{
		return m_rgb;
	}

	unsigned char b() const 
	{
		return (m_rgb & 0x00ff0000 ) >> 16;
	}

	unsigned char g() const 
	{
		return (m_rgb & 0x0000ff00 ) >> 8;
	}

	unsigned char r() const 
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

	inline Point()
		: m_x(0)
		, m_y(0)
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

	inline Size()
		: m_width(0)
		, m_height(0)
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
	
	inline Rect()  
		: origin()
		, size() 
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
	//
	// Tests that given point belongs to this object
	// 
	virtual bool hitTest(const Point& pt) = 0;

	//
	// Tests that given point located near the current object, 
	// most commonly to be used with touch up / btn up handlers, 
	// as they might threat event as 'to proceed' not only 
	// if the up event occurs withing the bounding rectangle, 
	// but also if its located not very far from it -- it is more handy  
	// in most cases 
	//
	virtual bool weakHitTest(const Point& pt) = 0;

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
	
	bool weakHitTest(const Point& pt)
	{
		IWidget* ret = NULL;

		for (  std::list<IWidget*>::const_iterator iter = m_listWidgets.begin();
			iter != m_listWidgets.end();
			++iter)
		{
			if ( (*iter)->weakHitTest(pt))
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

	virtual void hline(int x1, int x2, int y, const rgb& clr) = 0;
	
	virtual void vline(int x, int y1, int y2, const rgb& clr) = 0;

	virtual void drawImage(const Point& dstPos, const Image* srcImg, const Rect& srcRect, bool negative) = 0;

	virtual void invalidate() = 0;

	virtual void setBGColor(const rgb& color) = 0;

	virtual const rgb& getBGColor() = 0;

	virtual void setColor(const rgb& color) = 0;

	virtual const rgb& getColor() = 0;

	virtual void setBorderColor(const rgb& color) = 0;

	virtual const rgb& getBorderColor() = 0;
};


class BasicButton : public IWidget
{
	Rect m_visualRect;
	Rect m_activeRect;

	rgb m_color;
	rgb m_activeColor;
	
	IGraphics* m_gc;

	bool m_active;

protected:
	inline IGraphics* gc() const
	{
		return m_gc;
	}

	inline bool isActive() const
	{
		return m_active;
	}

	const Rect& visualRect() const 
	{
		return m_visualRect;
	}

	const Rect& activeRect() const
	{
		return m_activeRect;
	}

public:
	inline BasicButton(
			IGraphics* gc, 
			const Rect& visual, 
			const Rect& active 
			)
		: m_gc(gc)
		, m_visualRect(visual)
		, m_activeRect(active)
		, m_color(gc->getColor())
		, m_activeColor( m_color.negative() )
	{
	}
	
	void draw( )
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
	
	bool weakHitTest(const Point& pt)
	{
		int w = m_activeRect.getSize().getWidth();
		int h = m_activeRect.getSize().getHeight();
		int x = m_activeRect.getOrigin().getX();
		int y = m_activeRect.getOrigin().getY();

		Rect widerRect (x-w/2, y-h/2, w*2, h*2);

		return widerRect.inside(pt);
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

class ImageRscSet
{
	Image* m_image;

	std::map<int, Rect> m_rects;

public:
	ImageRscSet(Image *img)
		: m_image (img)
	{
	}

	void addRes(int id, const Rect& rect)
	{
		m_rects.insert(std::make_pair(id, rect));
	}

	const Rect& getRectForId(int id) const
	{
		static Rect stub(0,0,0,0);

		std::map<int, Rect>::const_iterator pos = m_rects.find(id);
		
		if ( pos != m_rects.end() ) 
		{
			return pos->second;
		}

		return stub;
	}

	const bool getRect(int id, Rect& outRect) const
	{
		bool ret = false;

		std::map<int, Rect>::const_iterator pos = m_rects.find(id);
		
		if ( pos != m_rects.end() ) 
		{
			outRect = pos->second;
			ret = true;
		}

		return ret;
	}
	
	Image* getImage() const
	{
		return m_image;
	}
};


class ImageButton : public BasicButton 
{
	std::vector<Point> m_dstImagePointsVect;
	
	std::vector<Image*> m_srcImagesVect;

	std::vector<Rect> m_srcImageRectsVect;

	int m_activeImage;

public:
	inline ImageButton(
			IGraphics* gc, 
			const Rect& visual, 
			const Rect& active, 
			const Point& imgBasePoint,
			Image* srcImg, 
			const Rect& srcImgRect
			)
		: BasicButton(gc, visual, active)
		, m_activeImage(0)
		, m_dstImagePointsVect( 1 ) // sz
		, m_srcImagesVect ( 1 ) // sz
		, m_srcImageRectsVect ( 1 ) // sz
	{
		m_dstImagePointsVect[0] = 
			Point( visual.getOrigin().getX() + imgBasePoint.getX(), 
		      	    	visual.getOrigin().getY() + imgBasePoint.getY() );

		m_srcImageRectsVect[0] = srcImgRect;

		m_srcImagesVect[0] = srcImg;
	}
	
	inline ImageButton(
			IGraphics* gc, 
			const Rect& visual, 
			const Rect& active, 
			const Point& imgBasePoint,
			ImageRscSet* rscSet, 
			int imgResId
			)
		: BasicButton(gc, visual, active)
		, m_activeImage(0)
		, m_dstImagePointsVect( 1 ) // sz
		, m_srcImagesVect ( 1 ) // sz
		, m_srcImageRectsVect ( 1 ) // sz
	{
		m_dstImagePointsVect[0] = 
			Point( visual.getOrigin().getX() + imgBasePoint.getX(), 
		      	    	visual.getOrigin().getY() + imgBasePoint.getY() );

		m_srcImageRectsVect[0] = rscSet->getRectForId(imgResId);

		m_srcImagesVect[0] = rscSet->getImage();

	}

	inline ImageButton(
			IGraphics* gc, 
			const Rect& visual, 
			const Rect& active, 
			const Point& imgBasePoint,
			ImageRscSet* rscSet, 
			int imgResId0,
			int imgResId1
			)
		: BasicButton(gc, visual, active)
		, m_activeImage(0)
		, m_dstImagePointsVect( 2 ) // sz
		, m_srcImagesVect ( 2 ) // sz
		, m_srcImageRectsVect ( 2 ) // sz
	{
		m_dstImagePointsVect[0] = 
			Point( visual.getOrigin().getX() + imgBasePoint.getX(), 
		      	    	visual.getOrigin().getY() + imgBasePoint.getY() );

		m_srcImageRectsVect[0] = rscSet->getRectForId(imgResId0);

		m_srcImagesVect[0] = rscSet->getImage();

		m_dstImagePointsVect[1] = 
			Point( visual.getOrigin().getX() + imgBasePoint.getX(), 
		      	    	visual.getOrigin().getY() + imgBasePoint.getY() );

		m_srcImageRectsVect[1] = rscSet->getRectForId(imgResId1);

		m_srcImagesVect[1] = rscSet->getImage();


	}

	inline ImageButton(
			IGraphics* gc, 
			const Rect& visual, 
			const Rect& active, 
			const std::vector<Point>& imgBasePoints,
			ImageRscSet* rscSet, 
			std::vector<int>& imgResIds
			)
		: BasicButton(gc, visual, active)
		, m_activeImage(0)
		, m_dstImagePointsVect( imgResIds.size() ) // sz
		, m_srcImagesVect ( imgResIds.size() ) // sz
		, m_srcImageRectsVect ( imgResIds.size() ) // sz
	{
		for (int i=0; i<imgResIds.size(); i++)
		{
			m_dstImagePointsVect[i] = 
				Point( visual.getOrigin().getX() + imgBasePoints[i].getX(), 
					visual.getOrigin().getY() + imgBasePoints[i].getY() );

			m_srcImageRectsVect[i] = rscSet->getRectForId(imgResIds[i]);

			m_srcImagesVect[i] = rscSet->getImage();
		}
	}

	void setActiveImage(int nr)
	{
		m_activeImage = nr % m_srcImageRectsVect.size();
		gc()->invalidate();
	}

	void draw( )
	{
		// fill the rectangle of btn with bg color first
		this->BasicButton::draw();

		if ( m_srcImagesVect[ m_activeImage ] ) 
		{
			bool invertColor = isActive();

			gc()->drawImage(
					m_dstImagePointsVect[m_activeImage], 
					m_srcImagesVect[m_activeImage], 
					m_srcImageRectsVect[m_activeImage], 
					invertColor 
				);

			const Rect& visual = visualRect();

			int x1 = visual.getOrigin().getX();
			int x2 = visual.getOrigin().getX() + visual.getSize().getWidth(); 
			int y1 = visual.getOrigin().getY(); 
			int y2 = visual.getOrigin().getY() + visual.getSize().getHeight();

			const rgb& clr = gc()->getBorderColor();

			gc()->hline(x1, x2, y1, clr );
			gc()->hline(x1, x2, y1+1, clr );
			gc()->hline(x1, x2, y2-1, clr );
			gc()->hline(x1, x2, y2-2, clr );
			gc()->vline(x1, y1, y2, clr );
			gc()->vline(x1+1, y1, y2, clr );
			gc()->vline(x2-1, y1, y2, clr );
			gc()->vline(x2-2, y1, y2, clr );
		}
	}
};



class TextEdit : public IWidget
{
	rgb m_color;
	
	IGraphics* m_gc;

	Point m_imgDrawOffset;

	ImageRscSet *m_fontDb;

	Rect m_rect;

	Size m_letterSize;

	int m_maxSize;

	std::string m_string;
	std::string m_displayString;

	bool m_password;

protected:
	inline IGraphics* gc() const
	{
		return m_gc;
	}

	const Rect& rect() const 
	{
		return m_rect;
	}

public:
	inline TextEdit( IGraphics* gc, Rect rect, Size ltrSize, Point offs, ImageRscSet* font, bool password)
		: m_color ( gc->getColor() ) 
		, m_gc ( gc )
		, m_rect ( rect )
		, m_letterSize ( ltrSize )
		, m_imgDrawOffset ( offs )
		, m_fontDb ( font )
		, m_password ( password )
	{
		m_maxSize =  m_rect.getSize().getWidth() / m_letterSize.getWidth();
	}

	inline void setString(const std::string& str)
	{
		m_string = str;

		if ( m_password ) 
		{
			m_displayString.clear();
			
			for (int i=0; i<m_string.size(); i++)
			{
				m_displayString += '*';
			}
		}
	}

	inline void appendChar(char c)
	{
		m_string += c;

		if ( m_password ) 
		{
			if ( m_displayString.size() )
			{
				m_displayString.erase(m_displayString.size()-1);
				m_displayString += '*';
			}
			m_displayString += c;
		}

		m_gc->invalidate();
	}

	inline void backspace()
	{
		if ( m_string.size() ) 
		{
			m_string.erase(m_string.size()-1);

			if ( m_password  && m_displayString.size() ) 
			{
				m_displayString.erase(m_displayString.size()-1);
			}
		}
	}

	inline void hideLastChr()
	{
		if ( m_displayString.size())
		{
			m_displayString.erase(m_displayString.size()-1);
			m_displayString += '*';
		}
	}

	bool hitTest(const Point& pt)
	{
		return m_rect.inside(pt); 
	}

	bool weakHitTest(const Point& pt)
	{
		int w = m_rect.getSize().getWidth();
		int h = m_rect.getSize().getHeight();
		int x = m_rect.getOrigin().getX();
		int y = m_rect.getOrigin().getY();

		Rect widerRect (x-w/2, y-h/2, w*2, h*2);

		return widerRect.inside(pt);
	}

	void onTouchDown(const Point& pt)
	{
	}

	void onTouchUp(const Point& pt)
	{
	}
	
	void onTouchUpdate(const Point& pt)
	{
	}

	void draw()
	{
		std::string substrToDisplay;

		size_t len = m_password ? m_displayString.size() : m_string.size();

		substrToDisplay = m_password ? m_displayString : m_string;
		
		if ( len >= m_maxSize )
		{
			substrToDisplay = substrToDisplay.substr(len - m_maxSize, m_maxSize);
		}

		m_gc->fill(m_rect, m_color);

		Point drawPos ( 
				m_rect.getOrigin().getX() + m_imgDrawOffset.getX(), 
				m_rect.getOrigin().getY() + m_imgDrawOffset.getY() 
			);

		Image *srcImg = m_fontDb->getImage();

		for (int i=0; i<substrToDisplay.size(); i++)
		{
			Rect srcRect;

			int chr = substrToDisplay[i];

			if ( m_fontDb->getRect(chr, srcRect ) )
			{
				m_gc->drawImage(
					drawPos, 
					srcImg, 
					srcRect, 
					false 
				);
			}

			drawPos.setX(
				drawPos.getX() + m_letterSize.getWidth() 
			);
			
		}

		const Rect& visual = m_rect;

		int x1 = visual.getOrigin().getX();
		int x2 = visual.getOrigin().getX() + visual.getSize().getWidth(); 
		int y1 = visual.getOrigin().getY(); 
		int y2 = visual.getOrigin().getY() + visual.getSize().getHeight();

		const rgb& clr = gc()->getBorderColor();

		gc()->hline(x1, x2, y1, clr );
		gc()->hline(x1, x2, y1+1, clr );
		gc()->hline(x1, x2, y2-1, clr );
		gc()->hline(x1, x2, y2-2, clr );
		gc()->vline(x1, y1, y2, clr );
		gc()->vline(x1+1, y1, y2, clr );
		gc()->vline(x2-1, y1, y2, clr );
		gc()->vline(x2-2, y1, y2, clr );

	}

	const std::string& getString() const
	{
		return m_string;
	}
};


#endif

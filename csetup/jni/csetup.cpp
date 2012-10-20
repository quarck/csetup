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

const char *touch_device = "/dev/input/event2";
const char *fb_device = "/dev/graphics/fb0";

// jump to the main init -- in furter builds, 
// now - just die
void escape(const char *message)
{
/*	FILE* log = fopen("/csetup/errlog", "a");
	if ( log ) 
	{
		fprintf(log, "%s\n", message);
		fclose(log);
	}
	else
	{
		printf("%s\n", message);
	} */
	printf("%s\n", message);
	exit(-1);
}

int x = 0;
int y = 0;

bool touch_active = false;
bool prev_touch_active = false;

int prev_code = -1;
int prev_type = -1;

bool quit = false;
bool needs_update = true;

void on_touch_down(int x, int y);
void on_touch_update(int x, int y);
void on_touch_up(int x, int y);

void report()
{
	if ( touch_active != prev_touch_active ) 
	{
		if ( touch_active )
		{
//			printf("DOWN: ");
			on_touch_down(x, y);
		}
		else
		{
			//printf("UP: ");
			on_touch_up(x, y);
//			quit = true;
		}
	}
	else
	{
		//printf("UPD: ");
		on_touch_update(x,y);
	}

	//printf("%d %d\n", x, y);
	fflush(stdout);

	prev_touch_active = touch_active;
}


class Point
{
	int m_x;
	int m_y;

public:
	int getX() const {return m_x; }
	void setX(int v) { m_x = v; }
	
	int getY() const {return m_y; }
	void setY(int v) { m_y = v; }
	
	inline Point(int x, int y) : m_x(x), m_y(y) {}
};

class Size
{
	int m_width;
	int m_height;

public:
	int getWidth() const {return m_width; }
	void setWidth(int v) { m_width = v; }
	
	int getHeight() const {return m_height; }
	void setHeight(int v) { m_height = v; }
	
	inline Size(int w, int h) : m_width(w), m_height(h) {}
};

class Rect
{
	Point origin;
	Size size;
	
public:

	const Point& getOrigin() const {return origin; }
	void setOrigin(Point& v) { origin = v; }

	const Size& getSize() const {return size; }
	void setSize(Size& v) { size = v; }
	
	inline Rect(int x, int y, int w, int h)  : origin(x,y), size(w,h) { }
	
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


class FrameBuffer
{
	int fd;

	fb_var_screeninfo vinfo;

	unsigned char *fb;

	int size;

	int bits_per_pixel;
	int bytes_per_pixel;

	int bytes_per_line;

	int max_buffers;

	int active_buffer;

public:
	const fb_var_screeninfo& info() const 
	{
		return vinfo;
	}

	const int numBuffers() const 
	{
		return max_buffers;
	}

	void setActiveBuffer(int nr)
	{
		active_buffer = nr;
	}

	void nextActiveBuffer()
	{
		active_buffer = (active_buffer + 1 ) % max_buffers;
	}

	const int xres() const 
	{
		return vinfo.xres;
	}

	const int yres() const 
	{
		return vinfo.yres;
	}


public:
	inline FrameBuffer(const char* dev)
		: fd ( -1 )
		, fb ( NULL )
		, size ( 0 )
		, active_buffer(0)
	{
		fd = open(dev, O_RDWR);

		if ( fd != -1 )
		{
    		if ( ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) != -1 ) 
			{

				//printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );

				bits_per_pixel = vinfo.bits_per_pixel;
				bytes_per_pixel = bits_per_pixel / 8;

				bytes_per_line = vinfo.xres_virtual * bytes_per_pixel;

				// Figure out the size of the screen in bytes
				size = vinfo.xres_virtual * vinfo.yres_virtual * bytes_per_pixel;

				//printf("%d %d %d\n", vinfo.xres_virtual, vinfo.yres_virtual, vinfo.yoffset);

				max_buffers = vinfo.yres_virtual / vinfo.yres;

				// Map the device to memory
				fb = (unsigned char *)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
			}
			else
			{
				perror("ioctl");
			}
		}
	}

	bool isValid() const
	{
		if ( fb != 0 && fb != (unsigned char*)(-1))
		{
			return true;
		}
		return false;
	}

	inline ~FrameBuffer()
	{
		if ( fb != NULL )
		{
			munmap(fb, size);
			fb = NULL;
		}

		if ( fd != -1 )
		{
			close(fd);
			fd = -1;
		}
	}

	inline void switchToBuf(int nr)
	{
		struct fb_var_screeninfo fb_var;
		ioctl(fd,FBIOGET_VSCREENINFO,&fb_var);
		fb_var.yoffset= fb_var.yres * nr;
		ioctl(fd,FBIOPAN_DISPLAY,&fb_var);
	}

	inline void switchToActiveBuf()
	{
		switchToBuf( active_buffer );
	}

	void fill(unsigned char r, unsigned char g, unsigned char b)
	{
		unsigned char *wptr = fb + (0/*y*/ + vinfo.yres * active_buffer)  *bytes_per_line + 0/*x*/ * bytes_per_pixel;

		for ( int y = 0; y < vinfo.yres; y ++ )
		{
			for ( int x = 0; x < vinfo.xres; x ++ )
			{
				*wptr++ = b;
				*wptr++ = g;
				*wptr++ = r;
				wptr++;
			}
		}
	}


	void fill(const Rect& area, unsigned char r, unsigned char g, unsigned char b)
	{

		for ( int y = 0; y < area.getSize().getHeight(); y ++ )
		{
			unsigned char *wptr = 
				fb 
				+ (area.getOrigin().getY() + y + vinfo.yres * active_buffer)  *bytes_per_line 
				+ area.getOrigin().getX() * bytes_per_pixel;

			for ( int x = 0; x < area.getSize().getWidth(); x ++ )
			{
				*wptr++ = b;
				*wptr++ = g;
				*wptr++ = r;
				wptr++;
			}
		}
	}	
	

	void mset(int c)
	{
		memset(fb, c, size);
	}

};

class Button
{
	Rect m_rect;
	
	int m_r, m_g, m_b;
	
	FrameBuffer* m_fb;
public:
	inline Button(FrameBuffer* fb, int x, int y, int w, int h, int r, int g, int b)
		: m_fb(fb)
		, m_rect(x,y,w,h)
		, m_r(r), m_g(g), m_b(b)
	{
	}
	
	inline void draw( bool active)
	{
		if ( !active ) 
		{
			m_fb->fill(m_rect, m_r, m_g, m_b);
		}
		else
		{
			m_fb->fill(m_rect, m_r, 255-m_g, 255-m_b);
		}
	}
	
	inline bool hitTest(const Point& pt)
	{
		return m_rect.inside(pt);
	}
	
	virtual void onDown(const Point& pt)
	{
		needs_update = true;
	}
	
	virtual void onUp(const Point& pt)
	{
		needs_update = true;
	}
};


class QuitButton : public Button
{
public:
	inline QuitButton(FrameBuffer* fb, int x, int y, int w, int h, int r, int g, int b)
		: Button(fb, x, y, w, h, r, g, b)
	{
	}

	void onUp(const Point& pt)
	{
		this->Button::onUp(pt);
		quit = true;
	}
};
	

std::list<Button*> buttons;
Button* activeButton = NULL;

void draw_all_btns()
{
	for (std::list<Button*>::iterator loIter = buttons.begin(); loIter != buttons.end(); ++ loIter )
	{
		(*loIter)->draw( activeButton == *loIter ? true : false);
	}
}

void on_touch_down(int x, int y)
{
	Point pt(x,y);
	
	for (std::list<Button*>::iterator loIter = buttons.begin(); loIter != buttons.end(); ++ loIter )
	{
		if ( (*loIter)->hitTest(pt) )
		{
			activeButton = *loIter;
			break;
		}
	}
	
	if ( activeButton )
	{
		activeButton->onDown(pt);
	}
}

void on_touch_update(int x, int y)
{
	Point pt(x,y);
	
	if ( activeButton )
	{
//		activeButton->draw(true);
	}
}

void on_touch_up(int x, int y)
{
	Point pt(x,y);
	
	if ( activeButton )
	{
		activeButton->onUp(pt);
		activeButton = NULL;
	}

}



int main(int argc, char *argv[])
{
	FrameBuffer fb(fb_device);

	if ( !fb.isValid() ) 
		escape("Failed to open frame buffer device");

	struct input_absinfo absx, absy;

	int touch = open(touch_device, O_RDONLY);

	if ( touch == -1 ) 
		escape("Failed to open touch screen device");


	if (ioctl(touch, EVIOCGABS(ABS_MT_POSITION_X), &absx) != 0 ) 
		escape("Faailed to get ioctl on touch screen device[1]");

	if (ioctl(touch, EVIOCGABS(ABS_MT_POSITION_Y), &absy) != 0 ) 
		escape("Failed to get ioctl on touch screen device[2]");

	fb.nextActiveBuffer();
	fb.fill(127, 127, 127);
	
	for (int ix = 0; ix < 6; ix ++ )
	{
		for (int iy = 0; iy < 10; iy ++ ) 
		{
			int px = ix * 80 + 40;
			int py = iy * 80 + 40;
			
			if ( ix == 0 && iy == 0 ) 
			{
				buttons.push_back(  
					new QuitButton(&fb, px, py, 60, 60, 255, 255, 255)
				);
			}
			else
			{
				buttons.push_back(  
					new Button(&fb, px, py, 60, 60, 255, 255, 255)
				);
			}
		}
	}

	draw_all_btns();

	fb.switchToActiveBuf();

	while (!quit)
	{
		if ( touch_active ) 
		{
			fd_set set;
			FD_ZERO(&set);
			FD_SET(touch, &set);

			timeval tv;

			tv.tv_sec = 0;
			tv.tv_usec = 100000;

			select( touch+1, &set, NULL, NULL, &tv);
			
			if ( ! FD_ISSET(touch, &set ) )
			{
				touch_active = false;
				report();
				
				if (needs_update ) 
				{
					fb.nextActiveBuffer();
					fb.fill(127, 127, 127);

					draw_all_btns();

					fb.switchToActiveBuf();
					needs_update = false;
				}

				continue;
			}
		}

		input_event ev;
		if ( read(touch, &ev, sizeof(ev)) != sizeof(ev))
		{
			escape("Read failed from the touch screen device");
		}

		switch (ev.type ) 
		{
		case EV_SYN: 
			switch (ev.code)
			{
			case SYN_REPORT:
//				if ( prev_type == EV_SYN && prev_code == SYN_MT_REPORT) 
//					break;
//				printf("syn-rep\n");
				report();
				break;
			case SYN_MT_REPORT:
//				if ( prev_type == EV_SYN && prev_code == SYN_REPORT) 
//					break;
//				printf("syn-mt-rep\n");
//				report();
				break;
			default:
//				printf("unk: EV_SYN: %d %d\n", ev.code, ev.value); 
				break;				
			}
			break;
		case EV_ABS:
			switch (ev.code) 
			{
			case ABS_X:
				//printf("abs: x %d\n", ev.value); 
				x = fb.xres() * (ev.value -absx.minimum) / (absx.maximum - absx.minimum + 1);
				touch_active = true;
				break;
			case ABS_Y:
				//printf("abs: y %d\n", ev.value); 
				y = fb.yres() * (ev.value -absy.minimum) / (absy.maximum - absy.minimum + 1);
				touch_active = true;
				break;
			case ABS_MT_SLOT:
				//printf("abs: mt_slot %d\n", ev.value); 
				break;
			case ABS_MT_POSITION_X:
				x = fb.xres() * (ev.value -absx.minimum) / (absx.maximum - absx.minimum + 1);
				touch_active = true;
				//printf("abs: mt_pos_x %d\n", ev.value); 
				break;
			case ABS_MT_POSITION_Y:
				y = fb.yres() * (ev.value -absy.minimum) / (absy.maximum - absy.minimum + 1);
				touch_active = true;
				//printf("abs: mt_pos_y %d\n", ev.value); 
				break;
			case ABS_MT_PRESSURE:
//				printf("abs: pressure %d\n", ev.value); 
				if ( ev.value == 0 )
					touch_active = 0;
				break;
			case ABS_MT_TOUCH_MAJOR:
				if ( ev.value == 0 ) 
					touch_active = 0;					
				//printf("abs: touch_major %d\n", ev.value); 
				break;
			case ABS_MT_TOUCH_MINOR:
				//printf("abs: touch_minor %d\n", ev.value); 
				break;
				
			default:
//				printf("unk: EV_ABS %x %x\n", ev.code, ev.value); 
				break;

			}
			break;
		case EV_KEY:
			switch (ev.code)
			{
			case BTN_TOUCH: 
				touch_active = ev.value ? true : false;
//				printf("btn-touch: %d\n", ev.value); 
				break;
			}
			break;
		default:
//			printf("unk: %d %d %d\n", ev.type, ev.code, ev.value); 
			break;

		}

		prev_type = ev.type;
		prev_code = ev.code;		


		if (needs_update ) 
		{
			fb.nextActiveBuffer();
			fb.fill(127, 127, 127);

			draw_all_btns();

			fb.switchToActiveBuf();
			needs_update = false;
		}
	}


	fb.nextActiveBuffer();
	fb.fill(0, 0, 0);
	fb.switchToActiveBuf();

	close(touch);

	return 0;
}


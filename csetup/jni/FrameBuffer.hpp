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


class FrameBuffer 
	: public IGraphics
{
	int m_fd;

	fb_var_screeninfo m_vinfo;

	unsigned char *m_fb;

	int m_size;

	int m_bits_per_pixel;
	int m_bytes_per_pixel;

	int m_bytes_per_line;

	int m_max_buffers;

	int m_active_buffer;

	bool m_needsUpdate;

public:
	const fb_var_screeninfo& info() const 
	{
		return m_vinfo;
	}

	const int numBuffers() const 
	{
		return m_max_buffers;
	}

	void setActiveBuffer(int nr)
	{
		m_active_buffer = nr;
	}

	void nextActiveBuffer()
	{
		m_active_buffer = (m_active_buffer + 1 ) % m_max_buffers;
	}

	const int xres() const 
	{
		return m_vinfo.xres;
	}

	const int yres() const 
	{
		return m_vinfo.yres;
	}


public:
	inline FrameBuffer(const char* dev)
		: m_fd ( -1 )
		, m_fb ( NULL )
		, m_size ( 0 )
		, m_active_buffer(0)
		, m_needsUpdate(false)
	{
		m_fd = open(dev, O_RDWR);

		if ( m_fd != -1 )
		{
			if ( ioctl(m_fd, FBIOGET_VSCREENINFO, &m_vinfo) != -1 ) 
			{
				m_bits_per_pixel = m_vinfo.bits_per_pixel;
				m_bytes_per_pixel = m_bits_per_pixel / 8;

				m_bytes_per_line = m_vinfo.xres_virtual * m_bytes_per_pixel;

				// Figure out the size of the screen in bytes
				m_size = m_vinfo.xres_virtual * m_vinfo.yres_virtual * m_bytes_per_pixel;

				m_max_buffers = m_vinfo.yres_virtual / m_vinfo.yres;

				// Map the device to memory
				m_fb = (unsigned char *)mmap(0, m_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
			}
		}
	}

	bool isValid() const
	{
		if ( m_fb != NULL && m_fb != (unsigned char*)(-1))
		{
			return true;
		}
		return false;
	}

	inline ~FrameBuffer()
	{
		if ( m_fb != NULL )
		{
			munmap(m_fb, m_size);
			m_fb = NULL;
		}

		if ( m_fd != -1 )
		{
			close(m_fd);
			m_fd = -1;
		}
	}

	inline void switchToBuf(int nr)
	{
		struct fb_var_screeninfo fb_var;
		ioctl(m_fd,FBIOGET_VSCREENINFO,&fb_var);
		fb_var.yoffset= fb_var.yres * nr;
		ioctl(m_fd,FBIOPAN_DISPLAY,&fb_var);
	}

	inline void switchToActiveBuf()
	{
		switchToBuf( m_active_buffer );
	}

	inline size_t offsetForPosition(int x, int y, int bufferIdx)
	{
		return ( y + m_vinfo.yres * bufferIdx ) * m_bytes_per_line 
			 + 
		  	x * m_bytes_per_pixel;
	}

	void fill(const rgb& p)
	{
		unsigned char *wptr = m_fb + offsetForPosition(0, 0, m_active_buffer);

		unsigned char r = p.r(), g = p.g(), b = p.b();

		for ( int y = 0; y < m_vinfo.yres; y ++ )
		{
			for ( int x = 0; x < m_vinfo.xres; x ++ )
			{
				*wptr++ = b;
				*wptr++ = g;
				*wptr++ = r;
				wptr++;
			}
		}
	}


	void fill(const Rect& area, const rgb& p)
	{
		unsigned char r = p.r(), g = p.g(), b = p.b();

		for ( int y = 0; y < area.getSize().getHeight(); y ++ )
		{
			unsigned char *wptr = 
				m_fb 
				+ 
				offsetForPosition(
						area.getOrigin().getX(), 
						area.getOrigin().getY() + y, 
						m_active_buffer
					);

			for ( int x = 0; x < area.getSize().getWidth(); x ++ )
			{
				*wptr++ = b;
				*wptr++ = g;
				*wptr++ = r;
				wptr++;
			}
		}
	}	
	
	void invalidate()
	{
		m_needsUpdate = true;
	}

	bool needsUpdate() const
	{
		return m_needsUpdate;
	}

	void setUpdated() 
	{
		m_needsUpdate = false;
	}

};




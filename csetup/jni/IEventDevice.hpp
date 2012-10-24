#ifndef __IEVENTDEVICE_HPP__
#define __IEVENTDEVICE_HPP__ 

class IEventDevice 
{
public:
	virtual int getReadFD() = 0;

	virtual int getWriteFD() = 0;

	virtual int getExcpFD() = 0;

	virtual void onFDReadReady() = 0;

	virtual void onFDWriteRead() = 0;

	virtual void onFDException() = 0;
};

#endif

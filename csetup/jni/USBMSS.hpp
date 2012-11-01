#ifndef __USB_MSS_HPP__
#define __USB_MSS_HPP__

#include "config.h"

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

#endif
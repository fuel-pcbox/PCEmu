#ifndef 808X_H
#define 808X_H

#include "common.h"

namespace CPU808X
{
	union reg16
	{
		struct
		{
			u8 l;
			u8 h;
		} parts;
		u16 w;
	};
	
	extern bool irq;
	extern u8 irqnum;
	
	extern reg16 axreg, bxreg, cxreg, dxreg;
	extern u16 si,di,bp,sp;
	extern u16 cs,ds,es,ss;
	extern u16 ip;
	
	void init();
	void tick();
}

#endif
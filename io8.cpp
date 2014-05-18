#include "io8.h"
#include "pic.h"

namespace IO8
{
u8 rb(u16 addr)
{
	return 0;
}

void wb(u16 addr, u8 data)
{
	//Motherboard emulation
	switch(addr)
	{
	case 0x20:
	case 0x21:
	case 0xA0:
	case 0xA1:
	{
		PIC::wb(addr,data);
		break;
	}
	}
}

}
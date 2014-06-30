#include "io8.h"
#include "pic.h"
#include "pit.h"
#include "kbdc.h"

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
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    {
        PIT::wb(addr,data);
    }
    case 0x61:
    {
        KBDC::wb(addr,data);
    }
    }
}

}
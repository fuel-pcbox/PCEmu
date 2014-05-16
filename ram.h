#ifndef RAM_H
#define RAM_H

#include "808x.h"

namespace RAM
{
extern u8 RAM[0x100000];

u8 rb(u16 seg, u16 off);

void wb(u16 seg, u16 off, u8 data);

}

#endif
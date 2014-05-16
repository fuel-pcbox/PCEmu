#ifndef IO_H
#define IO_H

#include "io8.h"

namespace IO
{
u8 rb(u16 addr);

u16 rw(u16 addr);

void wb(u16 addr, u8 data);

void ww(u16 addr, u16 data);

}

#endif
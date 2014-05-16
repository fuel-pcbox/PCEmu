#ifndef IO8_H
#define IO8_H

#include "808x.h"

namespace IO8
{
u8 rb(u16 addr);

void wb(u16 addr, u8 data);

}

#endif
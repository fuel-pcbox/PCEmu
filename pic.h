#ifndef PIC_H
#define PIC_H

#include "808x.h"

namespace PIC
{
void init();

u8 rb(u16 addr);

void wb(u16 addr, u8 data);

}

#endif
#ifndef PIT_H
#define PIT_H

#include "808x.h"

namespace PIT
{
void init();

void tick();

u8 rb(u16 addr);

void wb(u16 addr, u8 data);

}

#endif
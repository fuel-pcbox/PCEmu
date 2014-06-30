#ifndef KBDC_H
#define KBDC_H

#include "common.h"

namespace KBDC
{
extern u8 portb;

void init();

u8 rb(u16 addr);

void wb(u16 addr, u8 data);

}

#endif
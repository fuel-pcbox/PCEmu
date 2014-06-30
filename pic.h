#ifndef PIC_H
#define PIC_H

#include "808x.h"

namespace PIC
{

struct pic
{
    u8 imr, isr, irr;
    u8 offset;
    bool single;
    enum
    {
        ICW1,ICW2,ICW3,ICW4,READY
    } state;
};

extern pic pics[2];

void init();

u8 rb(u16 addr);

void wb(u16 addr, u8 data);

}

#endif

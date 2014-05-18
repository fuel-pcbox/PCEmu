#include "pic.h"

namespace PIC
{

struct pic
{
    u8 imr, isr, irr;
    u8 offset;
    bool single;
    enum
    {
        ICW1,ICW2,ICW3,ICW4,READY;
    } state;
};

pic pics[2];

void init()
{
    pics[0].imr = 0;
    pics[1].imr = 0;
    pics[0].state = ICW1;
    pics[1].state = ICW1;
}

u8 rb(u16 addr)
{
    switch(addr)
    {
    case 0x21:
    {
        return pics[0].imr;
        break;
    }
    case 0xA1:
    {
        return pics[1].imr;
        break;
    }
    }
}

void wb(u16 addr, u8 data)
{
    switch(addr)
    {
    case 0x20:
    {
        if(data & 0x10)
        {
            pics[0].imr = 0;
            pics[0].isr = 0;
            pics[0].irr = 0;
            
            pics[0].single = data & 2;
            
            pics[0].state = ICW2;
        }
        break;
    }
    case 0x21:
    {
        //TODO: MUCH OF THIS IS INACCURATE. FIX UP LATER.
        switch(pics[0].state)
        {
        case ICW2:
        {
            pics[0].offset = data;
            if(!pics[0].single) pics[0].state = ICW3;
            else pics[0].state = ICW4;
            break;
        }
        case ICW3:
        {
            pics[0].state = ICW4;
            break;
        }
        case ICW4:
        {
            pics[0].state = READY;
            break;
        }
        case READY:
        {
            pics[0].imr = data;
            break;
        }
        }
        break;
    }
    case 0xA0:
    {
        if(data & 0x10)
        {
            pics[1].imr = 0;
            pics[1].isr = 0;
            pics[1].irr = 0;
            
            pics[1].single = data & 2;
            
            pics[1].state = ICW2;
        }
        break;
    }
    case 0xA1:
    {
        //TODO: MUCH OF THIS IS INACCURATE. FIX UP LATER.
        switch(pics[1].state)
        {
        case ICW2:
        {
            pics[1].offset = data;
            if(!pics[1].single) pics[1].state = ICW3;
            else pics[1].state = ICW4;
            break;
        }
        case ICW3:
        {
            pics[1].state = ICW4;
            break;
        }
        case ICW4:
        {
            pics[1].state = READY;
            break;
        }
        case READY:
        {
            pics[1].imr = data;
            break;
        }
        }
        break;
    }
    }
}

}
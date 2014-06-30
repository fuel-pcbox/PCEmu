#include "pic.h"

namespace PIC
{
pic pics[2];

void init()
{
    pics[0].imr = 0;
    pics[1].imr = 0;
    pics[0].state = 0;
    pics[1].state = 0;
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

            pics[0].state = 1;
        }
        break;
    }
    case 0x21:
    {
        //TODO: MUCH OF THIS IS INACCURATE. FIX UP LATER.
        switch(pics[0].state)
        {
        case 1:
        {
            pics[0].offset = data;
            if(!pics[0].single) pics[0].state = 2;
            else pics[0].state = 3;
            break;
        }
        case 2:
        {
            pics[0].state = 3;
            break;
        }
        case 3:
        {
            pics[0].state = 4;
            break;
        }
        case 4:
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

            pics[1].state = 1;
        }
        break;
    }
    case 0xA1:
    {
        //TODO: MUCH OF THIS IS INACCURATE. FIX UP LATER.
        switch(pics[1].state)
        {
        case 1:
        {
            pics[1].offset = data;
            if(!pics[1].single) pics[1].state = 2;
            else pics[1].state = 3;
            break;
        }
        case 2:
        {
            pics[1].state = 3;
            break;
        }
        case 3:
        {
            pics[1].state = 4;
            break;
        }
        case 4:
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

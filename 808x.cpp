#include "808x.h"

namespace CPU808X
{
bool irq;
u8 irqnum;

reg16 axreg, bxreg, cxreg, dxreg;
u16 si,di,bp,sp;
u16 cs,ds,es,ss;
u16 ip;

enum segs
{
    SEG_DEFAULT,
    SEG_DS,
    SEG_CS,
    SEG_ES,
    SEG_SS
};

void init()
{
    cs = 0xFFFF;
    ip = 0;
}

struct locs
{
    union
    {
        u8* src8;
        u16* src16;
    };

    union
    {
        u8* dst8;
        u16* dst16;
    };
};

unsigned getaddr(u16 seg, u16 off)
{
    return (seg<<4)+off;
}

locs decodeops(int seg, u8 modrm, bool, word, bool segarg)
{
    locs res;
    int tmp = word | (segarg << 1);

    switch(modrm & 0xC7)
    {
    case 0x00:
    {
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bx+si)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bx+si)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bx+si)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bx+si)];
            break;
        }
        }
    }
    }

    return res;
}

void tick()
{
}
}
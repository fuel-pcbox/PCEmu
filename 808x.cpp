#include "common.h"
#include "808x.h"
#include "ram.h"
#include "io.h"

namespace CPU808X
{
//Interrupts
bool irq;
u8 irqnum;

//Registers
reg16 axreg, bxreg, cxreg, dxreg;
u16 si,di,bp,sp;
u16 cs,ds,es,ss;
u16 ip;

u16 flags;

//Useful for the HLT instruction.

bool halted;

//Prefix stuff.

enum segs
{
    SEG_DEFAULT,
    SEG_DS,
    SEG_CS,
    SEG_ES,
    SEG_SS
};

enum reps
{
    REP_NONE,
    REP_EQ,
    REP_NE
};

//CALL THIS BEFORE USING THE CPU.

void init()
{
    cs = 0xFFFF;
    ip = 0;
    flags = 0xF002;
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

locs decodeops(int seg, u8 modrm, bool word, bool segarg)
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
            res.src8 = &RAM::RAM[getaddr(ds,bxreg.w+si)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bxreg.w+si)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bxreg.w+si)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bxreg.w+si)];
            break;
        }
        }
    }
    case 0x01:
    {
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bxreg.w+di)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bxreg.w+di)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bxreg.w+di)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bxreg.w+di)];
            break;
        }
        }
    }
    case 0x02:
    {
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bp+si)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bp+si)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bp+si)];
            break;
        }
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bp+si)];
            break;
        }
        }
    }
    case 0x03:
    {
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bp+di)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bp+di)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bp+di)];
            break;
        }
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bp+di)];
            break;
        }
        }
    }
    case 0x04:
    {
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,si)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,si)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,si)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,si)];
            break;
        }
        }
    }
    case 0x05:
    {
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,di)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,di)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,di)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,di)];
            break;
        }
        }
    }
    case 0x06:
    {
        u16 tmp1 = RAM::rb(cs,ip+2)|(RAM::rb(cs,ip+3)<<8);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,tmp1)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,tmp1)];
            break;
        }
        }
    }
    case 0x07:
    {
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bxreg.w)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bxreg.w)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bxreg.w)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bxreg.w)];
            break;
        }
        }
    }
    case 0x40:
    {
        u8 tmp1 = RAM::rb(cs,ip+2);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bxreg.w+si+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bxreg.w+si+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bxreg.w+si+tmp1)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bxreg.w+si+tmp1)];
            break;
        }
        }
    }
    case 0x41:
    {
        u8 tmp1 = RAM::rb(cs,ip+2);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bxreg.w+di+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bxreg.w+di+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bxreg.w+di+tmp1)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bxreg.w+di+tmp1)];
            break;
        }
        }
    }
    case 0x42:
    {
        u8 tmp1 = RAM::rb(cs,ip+2);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bp+si+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bp+si+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bp+si+tmp1)];
            break;
        }
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bp+si+tmp1)];
            break;
        }
        }
    }
    case 0x43:
    {
        u8 tmp1 = RAM::rb(cs,ip+2);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bp+di+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bp+di+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bp+di+tmp1)];
            break;
        }
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bp+di+tmp1)];
            break;
        }
        }
    }
    case 0x44:
    {
        u8 tmp1 = RAM::rb(cs,ip+2);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,si+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,si+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,si+tmp1)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,si+tmp1)];
            break;
        }
        }
    }
    case 0x45:
    {
        u8 tmp1 = RAM::rb(cs,ip+2);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,di+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,di+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,di+tmp1)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,di+tmp1)];
            break;
        }
        }
    }
    case 0x46:
    {
        u8 tmp1 = RAM::rb(cs,ip+2);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bp+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bp+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bp+tmp1)];
            break;
        }
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bp+tmp1)];
            break;
        }
        }
    }
    case 0x47:
    {
        u8 tmp1 = RAM::rb(cs,ip+2);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bxreg.w+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bxreg.w+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bxreg.w+tmp1)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bxreg.w+tmp1)];
            break;
        }
        }
    }
    case 0x80:
    {
        u16 tmp1 = RAM::rb(cs,ip+2) | (RAM::rb(cs,ip+3)<<8);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bxreg.w+si+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bxreg.w+si+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bxreg.w+si+tmp1)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bxreg.w+si+tmp1)];
            break;
        }
        }
    }
    case 0x81:
    {
        u16 tmp1 = RAM::rb(cs,ip+2) | (RAM::rb(cs,ip+3)<<8);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bxreg.w+di+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bxreg.w+di+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bxreg.w+di+tmp1)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bxreg.w+di+tmp1)];
            break;
        }
        }
    }
    case 0x82:
    {
        u16 tmp1 = RAM::rb(cs,ip+2) | (RAM::rb(cs,ip+3)<<8);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bp+si+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bp+si+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bp+si+tmp1)];
            break;
        }
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bp+si+tmp1)];
            break;
        }
        }
    }
    case 0x83:
    {
        u16 tmp1 = RAM::rb(cs,ip+2) | (RAM::rb(cs,ip+3)<<8);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bp+di+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bp+di+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bp+di+tmp1)];
            break;
        }
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bp+di+tmp1)];
            break;
        }
        }
    }
    case 0x84:
    {
        u16 tmp1 = RAM::rb(cs,ip+2) | (RAM::rb(cs,ip+3)<<8);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,si+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,si+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,si+tmp1)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,si+tmp1)];
            break;
        }
        }
    }
    case 0x85:
    {
        u16 tmp1 = RAM::rb(cs,ip+2) | (RAM::rb(cs,ip+3)<<8);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,di+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,di+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,di+tmp1)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,di+tmp1)];
            break;
        }
        }
    }
    case 0x86:
    {
        u16 tmp1 = RAM::rb(cs,ip+2) | (RAM::rb(cs,ip+3)<<8);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bp+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bp+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bp+tmp1)];
            break;
        }
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bp+tmp1)];
            break;
        }
        }
    }
    case 0x87:
    {
        u16 tmp1 = RAM::rb(cs,ip+2) | (RAM::rb(cs,ip+3)<<8);
        switch(seg)
        {
        case SEG_DEFAULT:
        case SEG_DS:
        {
            res.src8 = &RAM::RAM[getaddr(ds,bxreg.w+tmp1)];
            break;
        }
        case SEG_CS:
        {
            res.src8 = &RAM::RAM[getaddr(cs,bxreg.w+tmp1)];
            break;
        }
        case SEG_ES:
        {
            res.src8 = &RAM::RAM[getaddr(es,bxreg.w+tmp1)];
            break;
        }
        case SEG_SS:
        {
            res.src8 = &RAM::RAM[getaddr(ss,bxreg.w+tmp1)];
            break;
        }
        }
    }
    case 0xC0:
    {
        switch(tmp&1)
        {
        case 0:
        {
            res.src8 = &axreg.parts.l;
            break;
        }
        case 1:
        {
            res.src16 = &axreg.w;
        }
        }
        break;
    }
    case 0xC1:
    {
        switch(tmp&1)
        {
        case 0:
        {
            res.src8 = &cxreg.parts.l;
            break;
        }
        case 1:
        {
            res.src16 = &cxreg.w;
        }
        }
        break;
    }
    case 0xC2:
    {
        switch(tmp&1)
        {
        case 0:
        {
            res.src8 = &dxreg.parts.l;
            break;
        }
        case 1:
        {
            res.src16 = &dxreg.w;
        }
        }
        break;
    }
    case 0xC3:
    {
        switch(tmp&1)
        {
        case 0:
        {
            res.src8 = &bxreg.parts.l;
            break;
        }
        case 1:
        {
            res.src16 = &bxreg.w;
        }
        }
        break;
    }
    case 0xC4:
    {
        switch(tmp&1)
        {
        case 0:
        {
            res.src8 = &axreg.parts.h;
            break;
        }
        case 1:
        {
            res.src16 = &sp;
        }
        }
        break;
    }
    case 0xC5:
    {
        switch(tmp&1)
        {
        case 0:
        {
            res.src8 = &cxreg.parts.h;
            break;
        }
        case 1:
        {
            res.src16 = &bp;
        }
        }
        break;
    }
    case 0xC6:
    {
        switch(tmp&1)
        {
        case 0:
        {
            res.src8 = &dxreg.parts.h;
            break;
        }
        case 1:
        {
            res.src16 = &si;
        }
        }
        break;
    }
    case 0xC7:
    {
        switch(tmp&1)
        {
        case 0:
        {
            res.src8 = &bxreg.parts.h;
            break;
        }
        case 1:
        {
            res.src16 = &di;
        }
        }
        break;
    }
    }

    switch(modrm & 0x38)
    {
    case 0x00:
    {
        switch(tmp)
        {
        case 0:
        {
            res.dst8 = &axreg.parts.l;
            break;
        }
        case 1:
        {
            res.dst16 = &axreg.w;
            break;
        }
        case 3:
        {
            res.dst16 = &es;
        }
        }
        break;
    }
    case 0x08:
    {
        switch(tmp)
        {
        case 0:
        {
            res.dst8 = &cxreg.parts.l;
            break;
        }
        case 1:
        {
            res.dst16 = &cxreg.w;
            break;
        }
        case 3:
        {
            res.dst16 = &cs;
        }
        }
        break;
    }
    case 0x10:
    {
        switch(tmp)
        {
        case 0:
        {
            res.dst8 = &dxreg.parts.l;
            break;
        }
        case 1:
        {
            res.dst16 = &dxreg.w;
            break;
        }
        case 3:
        {
            res.dst16 = &ss;
        }
        }
        break;
    }
    case 0x18:
    {
        switch(tmp)
        {
        case 0:
        {
            res.dst8 = &bxreg.parts.l;
            break;
        }
        case 1:
        {
            res.dst16 = &bxreg.w;
            break;
        }
        case 3:
        {
            res.dst16 = &ds;
        }
        }
        break;
    }
    case 0x20:
    {
        switch(tmp)
        {
        case 0:
        {
            res.dst8 = &axreg.parts.h;
            break;
        }
        case 1:
        {
            res.dst16 = &sp;
            break;
        }
        }
        break;
    }
    case 0x28:
    {
        switch(tmp)
        {
        case 0:
        {
            res.dst8 = &cxreg.parts.h;
            break;
        }
        case 1:
        {
            res.dst16 = &bp;
            break;
        }
        }
        break;
    }
    case 0x30:
    {
        switch(tmp)
        {
        case 0:
        {
            res.dst8 = &dxreg.parts.h;
            break;
        }
        case 1:
        {
            res.dst16 = &si;
            break;
        }
        }
        break;
    }
    case 0x38:
    {
        switch(tmp)
        {
        case 0:
        {
            res.dst8 = &bxreg.parts.h;
            break;
        }
        case 1:
        {
            res.dst16 = &di;
            break;
        }
        }
        break;
    }
    }

    if(modrm < 0xC0)
    {
        if((modrm & 0xC7) == 0x06) ip+=2;
        if(modrm & 0x40) ip+=1;
        else if(modrm >= 0x80) ip+=2;
    }

    return res;
}

int seg;
int rep;

//Flag handling functions.

inline void handleZ(u16 val)
{
    if(val == 0) flags |= 0x0040;
    else flags &= 0xFFBF;
}

inline void handleS(u16 val, bool word)
{
    if(word)
    {
        if(val & 0x8000) flags |= 0x0080;
        else flags &= 0xFF7F;
    }
    else
    {
        if(val & 0x80) flags |= 0x0080;
        else flags &= 0xFF7F;
    }
}

inline void handleOAdd(u16 val1, u16 val2, bool word)
{
    if(word)
    {
        if((((val1 ^ val2) ^ 0x8000) & ((val1 + val2) ^ val1) & 0x8000)) flags |= 0x0800;
        else flags &= 0xF7FF;
    }
    else
    {
        if((((val1 ^ val2) ^ 0x80) & ((val1 + val2) ^ val1) & 0x80)) flags |= 0x0800;
        else flags &= 0xF7FF;
    }
}

inline void handleOSub(u16 val1, u16 val2, bool word)
{
    if(word)
    {
        if((((val1 ^ val2) ^ 0x8000) & ((val1 - val2) ^ val1) & 0x8000)) flags |= 0x0800;
        else flags &= 0xF7FF;
    }
    else
    {
        if((((val1 ^ val2) ^ 0x80) & ((val1 - val2) ^ val1) & 0x80)) flags |= 0x0800;
        else flags &= 0xF7FF;
    }
}

inline void handleOAnd(u16 val1, u16 val2, bool word)
{
    if(word)
    {
        if((((val1 ^ val2) ^ 0x8000) & ((val1 & val2) ^ val1) & 0x8000)) flags |= 0x0800;
        else flags &= 0xF7FF;
    }
    else
    {
        if((((val1 ^ val2) ^ 0x80) & ((val1 & val2) ^ val1) & 0x80)) flags |= 0x0800;
        else flags &= 0xF7FF;
    }
}

inline void handleOXor(u16 val1, u16 val2, bool word)
{
    if(word)
    {
        if((((val1 ^ val2) ^ 0x8000) & ((val1 ^ val2) ^ val1) & 0x8000)) flags |= 0x0800;
        else flags &= 0xF7FF;
    }
    else
    {
        if((((val1 ^ val2) ^ 0x80) & ((val1 ^ val2) ^ val1) & 0x80)) flags |= 0x0800;
        else flags &= 0xF7FF;
    }
}

inline void handleP(u16 val, bool word)
{
    if(word)
    {
        u8 v = 0;
        for(int i = 0; i < 16; i++)
        {
            if(val & (1 << i)) v ^= 1;
        }
        if(!v) flags |= 0x0004;
        else flags &= 0xFFFB;
    }
    else
    {
        u8 v = 0;
        for(int i = 0; i < 8; i++)
        {
            if(val & (1 << i)) v ^= 1;
        }
        if(!v) flags |= 0x0004;
        else flags &= 0xFFFB;
    }
}

inline void handleC(u32 val, bool word)
{
    if(word)
    {
        if(val >= 0x10000) flags |= 0x0001;
        else flags &= 0xFFFE;
    }
    else
    {
        if(val >= 0x100) flags |= 0x0001;
        else flags &= 0xFFFE;
    }
}

inline void handleA(u16 val1, u16 val2)
{
    if((val1 & 0x10) ^ (val2 & 0x10)) flags |= 0x0010;
    else flags &= 0xFFEF;
}

void interrupt(u8 intr)
{
}

//Only documented opcodes, unless they're widely used. (Which, on the 8086, they AREN'T.)

void rtick()
{
    u8 op = RAM::rb(cs,ip);
    printf("%02X\n",op);
    if(!halted)
    {
        switch(op)
        {
        case 0x00:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.src8;
            u8 tmp1 = *loc.dst8;
            u16 tmp2 = tmp + tmp1;
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOAdd(tmp,tmp1,false);
            *loc.src8 += *loc.dst8;
            u8 tmp3 = *loc.src8;
            handleP(tmp3,false);
            handleZ(tmp3);
            handleS(tmp3,false);
            ip+=2;
            break;
        }
        case 0x01:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.src16;
            u16 tmp1 = *loc.dst16;
            u32 tmp2 = tmp + tmp1;
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOAdd(tmp,tmp1,true);
            *loc.src16 += *loc.dst16;
            u16 tmp3 = *loc.src16;
            handleP(tmp3,true);
            handleZ(tmp3);
            handleS(tmp3,true);
            ip+=2;
            break;
        }
        case 0x02:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.dst8;
            u8 tmp1 = *loc.src8;
            u16 tmp2 = tmp + tmp1;
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOAdd(tmp1,tmp,false);
            *loc.dst8 += *loc.src8;
            u8 tmp3 = *loc.dst8;
            handleP(tmp3,false);
            handleZ(tmp3);
            handleS(tmp3,false);
            ip+=2;
            break;
        }
        case 0x03:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.dst16;
            u16 tmp1 = *loc.src16;
            u32 tmp2 = tmp + tmp1;
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOAdd(tmp1,tmp,true);
            *loc.dst16 += *loc.src16;
            u16 tmp3 = *loc.dst16;
            handleP(tmp3,true);
            handleZ(tmp3);
            handleS(tmp3,true);
            ip+=2;
            break;
        }
        case 0x04:
        {
            u8 tmp = axreg.parts.l;
            u8 tmp1 = RAM::rb(cs,ip+1);
            u16 tmp2 = tmp + tmp1;
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOAdd(tmp,tmp1,false);
            axreg.parts.l += tmp1;
            handleP(axreg.parts.l,false);
            handleZ(axreg.parts.l);
            handleS(axreg.parts.l,false);
            ip+=2;
            break;
        }
        case 0x05:
        {
            u16 tmp = axreg.w;
            u16 tmp1 = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            u32 tmp2 = tmp + tmp1;
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOAdd(tmp,tmp1,true);
            axreg.w += tmp1;
            handleP(axreg.w,true);
            handleZ(axreg.w);
            handleS(axreg.w,true);
            ip+=3;
            break;
        }
        case 0x06:
        {
            sp-=2;
            RAM::wb(ss,sp,es);
            ip+=1;
            break;
        }
        case 0x07:
        {
            es = RAM::rb(ss,sp);
            sp+=2;
            ip+=1;
            break;
        }
        case 0x08:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.src8;
            u8 tmp1 = *loc.dst8;
            flags &= 0xF7FE;
            *loc.src8 |= *loc.dst8;
            u8 tmp3 = *loc.src8;
            handleP(tmp3,false);
            handleZ(tmp3);
            handleS(tmp3,false);
            ip+=2;
            break;
        }
        case 0x09:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.src16;
            u16 tmp1 = *loc.dst16;
            flags &= 0xF7FE;
            *loc.src16 |= *loc.dst16;
            u16 tmp3 = *loc.src16;
            handleP(tmp3,true);
            handleZ(tmp3);
            handleS(tmp3,true);
            ip+=2;
            break;
        }
        case 0x0A:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.dst8;
            u8 tmp1 = *loc.src8;
            flags &= 0xF7FE;
            *loc.dst8 |= *loc.src8;
            u8 tmp3 = *loc.dst8;
            handleP(tmp3,false);
            handleZ(tmp3);
            handleS(tmp3,false);
            ip+=2;
            break;
        }
        case 0x0B:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.dst16;
            u16 tmp1 = *loc.src16;
            flags &= 0xF7FE;
            *loc.dst16 |= *loc.src16;
            u16 tmp3 = *loc.dst16;
            handleP(tmp3,true);
            handleZ(tmp3);
            handleS(tmp3,true);
            ip+=2;
            break;
        }
        case 0x0C:
        {
            u8 tmp = axreg.parts.l;
            u8 tmp1 = RAM::rb(cs,ip+1);
            flags &= 0xF7FE;
            axreg.parts.l |= tmp1;
            handleP(axreg.parts.l,false);
            handleZ(axreg.parts.l);
            handleS(axreg.parts.l,false);
            ip+=2;
            break;
        }
        case 0x0D:
        {
            u16 tmp = axreg.w;
            u16 tmp1 = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            flags &= 0xF7FE;
            axreg.w |= tmp1;
            handleP(axreg.w,true);
            handleZ(axreg.w);
            handleS(axreg.w,true);
            ip+=3;
            break;
        }
        case 0x0E:
        {
            sp-=2;
            RAM::wb(ss,sp,cs);
            ip+=1;
            break;
        }
        //0x0F is undocumented on the 8086.
        case 0x10:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.src8;
            u8 tmp1 = *loc.dst8;
            u16 tmp2 = tmp + tmp1 + (flags & 1);
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOAdd(tmp,tmp1,false);
            *loc.src8 += *loc.dst8 + (flags & 1);
            u8 tmp3 = *loc.src8;
            handleP(tmp3,false);
            handleZ(tmp3);
            handleS(tmp3,false);
            ip+=2;
            break;
        }
        case 0x11:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.src16;
            u16 tmp1 = *loc.dst16;
            u32 tmp2 = tmp + tmp1 + (flags & 1);
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOAdd(tmp,tmp1,true);
            *loc.src16 += *loc.dst16 + (flags & 1);
            u16 tmp3 = *loc.src16;
            handleP(tmp3,true);
            handleZ(tmp3);
            handleS(tmp3,true);
            ip+=2;
            break;
        }
        case 0x12:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.dst8;
            u8 tmp1 = *loc.src8;
            u16 tmp2 = tmp + tmp1 + (flags & 1);
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOAdd(tmp1,tmp,false);
            *loc.dst8 += *loc.src8 + (flags & 1);
            u8 tmp3 = *loc.dst8;
            handleP(tmp3,false);
            handleZ(tmp3);
            handleS(tmp3,false);
            ip+=2;
            break;
        }
        case 0x13:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.dst16;
            u16 tmp1 = *loc.src16;
            u32 tmp2 = tmp + tmp1 + (flags & 1);
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOAdd(tmp1,tmp,true);
            *loc.dst16 += *loc.src16 + (flags & 1);
            u16 tmp3 = *loc.dst16;
            handleP(tmp3,true);
            handleZ(tmp3);
            handleS(tmp3,true);
            ip+=2;
            break;
        }
        case 0x14:
        {
            u8 tmp = axreg.parts.l;
            u8 tmp1 = RAM::rb(cs,ip+1);
            u16 tmp2 = tmp + tmp1 + (flags & 1);
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOAdd(tmp,tmp1,false);
            axreg.parts.l += tmp1 + (flags & 1);
            handleP(axreg.parts.l,false);
            handleZ(axreg.parts.l);
            handleS(axreg.parts.l,false);
            ip+=2;
            break;
        }
        case 0x15:
        {
            u16 tmp = axreg.w;
            u16 tmp1 = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            u32 tmp2 = tmp + tmp1 + (flags & 1);
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOAdd(tmp,tmp1,true);
            axreg.w += tmp1 + (flags & 1);
            handleP(axreg.w,true);
            handleZ(axreg.w);
            handleS(axreg.w,true);
            ip+=3;
            break;
        }
        case 0x16:
        {
            sp-=2;
            RAM::wb(ss,sp,ss);
            ip+=1;
            break;
        }
        case 0x17:
        {
            ss = RAM::rb(ss,sp);
            sp+=2;
            ip+=1;
            break;
        }
        case 0x18:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.src8;
            u8 tmp1 = *loc.dst8;
            u16 tmp2 = tmp - tmp1 - (flags & 1);
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOSub(tmp,tmp1,false);
            *loc.src8 -= *loc.dst8 + (flags & 1);
            u8 tmp3 = *loc.src8;
            handleP(tmp3,false);
            handleZ(tmp3);
            handleS(tmp3,false);
            ip+=2;
            break;
        }
        case 0x19:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.src16;
            u16 tmp1 = *loc.dst16;
            u32 tmp2 = tmp - tmp1 - (flags & 1);
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOSub(tmp,tmp1,true);
            *loc.src16 -= *loc.dst16 + (flags & 1);
            u16 tmp3 = *loc.src16;
            handleP(tmp3,true);
            handleZ(tmp3);
            handleS(tmp3,true);
            ip+=2;
            break;
        }
        case 0x1A:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.dst8;
            u8 tmp1 = *loc.src8;
            u16 tmp2 = tmp - tmp1 - (flags & 1);
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOSub(tmp1,tmp,false);
            *loc.dst8 -= *loc.src8 + (flags & 1);
            u8 tmp3 = *loc.dst8;
            handleP(tmp3,false);
            handleZ(tmp3);
            handleS(tmp3,false);
            ip+=2;
            break;
        }
        case 0x1B:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.dst16;
            u16 tmp1 = *loc.src16;
            u32 tmp2 = tmp - tmp1 - (flags & 1);
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOSub(tmp1,tmp,true);
            *loc.dst16 -= *loc.src16 + (flags & 1);
            u16 tmp3 = *loc.dst16;
            handleP(tmp3,true);
            handleZ(tmp3);
            handleS(tmp3,true);
            ip+=2;
            break;
        }
        case 0x1C:
        {
            u8 tmp = axreg.parts.l;
            u8 tmp1 = RAM::rb(cs,ip+1);
            u16 tmp2 = tmp - tmp1 - (flags & 1);
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOSub(tmp,tmp1,false);
            axreg.parts.l -= tmp1 + (flags & 1);
            handleP(axreg.parts.l,false);
            handleZ(axreg.parts.l);
            handleS(axreg.parts.l,false);
            ip+=2;
            break;
        }
        case 0x1D:
        {
            u16 tmp = axreg.w;
            u16 tmp1 = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            u32 tmp2 = tmp - tmp1 - (flags & 1);
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOSub(tmp,tmp1,true);
            axreg.w -= tmp1 + (flags & 1);
            handleP(axreg.w,true);
            handleZ(axreg.w);
            handleS(axreg.w,true);
            ip+=3;
            break;
        }
        case 0x1E:
        {
            sp-=2;
            RAM::wb(ss,sp,ds);
            ip+=1;
            break;
        }
        case 0x1F:
        {
            ds = RAM::rb(ss,sp);
            sp+=2;
            ip+=1;
            break;
        }
        case 0x20:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.src8;
            u8 tmp1 = *loc.dst8;
            u16 tmp2 = tmp & tmp1;
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOAnd(tmp,tmp1,false);
            *loc.src8 &= *loc.dst8;
            u8 tmp3 = *loc.src8;
            handleP(tmp3,false);
            handleZ(tmp3);
            handleS(tmp3,false);
            ip+=2;
            break;
        }
        case 0x21:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.src16;
            u16 tmp1 = *loc.dst16;
            u32 tmp2 = tmp & tmp1;
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOAnd(tmp,tmp1,true);
            *loc.src16 &= *loc.dst16;
            u16 tmp3 = *loc.src16;
            handleP(tmp3,true);
            handleZ(tmp3);
            handleS(tmp3,true);
            ip+=2;
            break;
        }
        case 0x22:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.dst8;
            u8 tmp1 = *loc.src8;
            u16 tmp2 = tmp & tmp1;
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOAnd(tmp1,tmp,false);
            *loc.dst8 &= *loc.src8;
            u8 tmp3 = *loc.dst8;
            handleP(tmp3,false);
            handleZ(tmp3);
            handleS(tmp3,false);
            ip+=2;
            break;
        }
        case 0x23:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.dst16;
            u16 tmp1 = *loc.src16;
            u32 tmp2 = tmp & tmp1;
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOAnd(tmp1,tmp,true);
            *loc.dst16 &= *loc.src16;
            u16 tmp3 = *loc.dst16;
            handleP(tmp3,true);
            handleZ(tmp3);
            handleS(tmp3,true);
            ip+=2;
            break;
        }
        case 0x24:
        {
            u8 tmp = axreg.parts.l;
            u8 tmp1 = RAM::rb(cs,ip+1);
            u16 tmp2 = tmp & tmp1;
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOAnd(tmp,tmp1,false);
            axreg.parts.l &= tmp1;
            handleP(axreg.parts.l,false);
            handleZ(axreg.parts.l);
            handleS(axreg.parts.l,false);
            ip+=2;
            break;
        }
        case 0x25:
        {
            u16 tmp = axreg.w;
            u16 tmp1 = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            u32 tmp2 = tmp & tmp1;
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOAnd(tmp,tmp1,true);
            axreg.w &= tmp1;
            handleP(axreg.w,true);
            handleZ(axreg.w);
            handleS(axreg.w,true);
            ip+=3;
            break;
        }
        case 0x28:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.src8;
            u8 tmp1 = *loc.dst8;
            u16 tmp2 = tmp - tmp1;
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOSub(tmp,tmp1,false);
            *loc.src8 -= *loc.dst8;
            u8 tmp3 = *loc.src8;
            handleP(tmp3,false);
            handleZ(tmp3);
            handleS(tmp3,false);
            ip+=2;
            break;
        }
        case 0x29:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.src16;
            u16 tmp1 = *loc.dst16;
            u32 tmp2 = tmp - tmp1;
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOSub(tmp,tmp1,true);
            *loc.src16 -= *loc.dst16;
            u16 tmp3 = *loc.src16;
            handleP(tmp3,true);
            handleZ(tmp3);
            handleS(tmp3,true);
            ip+=2;
            break;
        }
        case 0x2A:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.dst8;
            u8 tmp1 = *loc.src8;
            u16 tmp2 = tmp - tmp1;
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOSub(tmp1,tmp,false);
            *loc.dst8 -= *loc.src8;
            u8 tmp3 = *loc.dst8;
            handleP(tmp3,false);
            handleZ(tmp3);
            handleS(tmp3,false);
            ip+=2;
            break;
        }
        case 0x2B:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.dst16;
            u16 tmp1 = *loc.src16;
            u32 tmp2 = tmp - tmp1;
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOSub(tmp1,tmp,true);
            *loc.dst16 -= *loc.src16;
            u16 tmp3 = *loc.dst16;
            handleP(tmp3,true);
            handleZ(tmp3);
            handleS(tmp3,true);
            ip+=2;
            break;
        }
        case 0x2C:
        {
            u8 tmp = axreg.parts.l;
            u8 tmp1 = RAM::rb(cs,ip+1);
            u16 tmp2 = tmp - tmp1;
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOSub(tmp,tmp1,false);
            axreg.parts.l -= tmp1;
            handleP(axreg.parts.l,false);
            handleZ(axreg.parts.l);
            handleS(axreg.parts.l,false);
            ip+=2;
            break;
        }
        case 0x2D:
        {
            u16 tmp = axreg.w;
            u16 tmp1 = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            u32 tmp2 = tmp - tmp1;
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOSub(tmp,tmp1,true);
            axreg.w -= tmp1;
            handleP(axreg.w,true);
            handleZ(axreg.w);
            handleS(axreg.w,true);
            ip+=3;
            break;
        }
        case 0x30:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.src8;
            u8 tmp1 = *loc.dst8;
            u16 tmp2 = tmp ^ tmp1;
            handleC(tmp2,false);
            handleOXor(tmp,tmp1,false);
            *loc.src8 ^= *loc.dst8;
            u8 tmp3 = *loc.src8;
            handleP(tmp3,false);
            handleZ(tmp3);
            handleS(tmp3,false);
            ip+=2;
            break;
        }
        case 0x31:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.src16;
            u16 tmp1 = *loc.dst16;
            u32 tmp2 = tmp ^ tmp1;
            handleC(tmp2,true);
            handleOXor(tmp,tmp1,true);
            *loc.src16 ^= *loc.dst16;
            u16 tmp3 = *loc.src16;
            handleP(tmp3,true);
            handleZ(tmp3);
            handleS(tmp3,true);
            ip+=2;
            break;
        }
        case 0x32:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.dst8;
            u8 tmp1 = *loc.src8;
            u16 tmp2 = tmp ^ tmp1;
            handleC(tmp2,false);
            handleOXor(tmp1,tmp,false);
            *loc.dst8 ^= *loc.src8;
            u8 tmp3 = *loc.dst8;
            handleP(tmp3,false);
            handleZ(tmp3);
            handleS(tmp3,false);
            ip+=2;
            break;
        }
        case 0x33:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.dst16;
            u16 tmp1 = *loc.src16;
            u32 tmp2 = tmp ^ tmp1;
            handleC(tmp2,true);
            handleOXor(tmp1,tmp,true);
            *loc.dst16 ^= *loc.src16;
            u16 tmp3 = *loc.dst16;
            handleP(tmp3,true);
            handleZ(tmp3);
            handleS(tmp3,true);
            ip+=2;
            break;
        }
        case 0x34:
        {
            u8 tmp = axreg.parts.l;
            u8 tmp1 = RAM::rb(cs,ip+1);
            u16 tmp2 = tmp ^ tmp1;
            handleC(tmp2,false);
            handleOXor(tmp,tmp1,false);
            axreg.parts.l ^= tmp1;
            handleP(axreg.parts.l,false);
            handleZ(axreg.parts.l);
            handleS(axreg.parts.l,false);
            ip+=2;
            break;
        }
        case 0x35:
        {
            u16 tmp = axreg.w;
            u16 tmp1 = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            u32 tmp2 = tmp ^ tmp1;
            handleC(tmp2,true); 
            handleOXor(tmp,tmp1,true);
            axreg.w ^= tmp1;
            handleP(axreg.w,true);
            handleZ(axreg.w);
            handleS(axreg.w,true);
            ip+=3;
            break;
        }
        case 0x38:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.src8;
            u8 tmp1 = *loc.dst8;
            u16 tmp2 = tmp - tmp1;
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOSub(tmp,tmp1,false);
            handleP(tmp2&0xFF,false);
            handleZ(tmp2&0xFF);
            handleS(tmp2&0xFF,false);
            ip+=2;
            break;
        }
        case 0x39:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.src16;
            u16 tmp1 = *loc.dst16;
            u32 tmp2 = tmp - tmp1;
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOSub(tmp,tmp1,true);
            handleP(tmp2&0xFFFF,true);
            handleZ(tmp2&0xFFFF);
            handleS(tmp2&0xFFFF,true);
            ip+=2;
            break;
        }
        case 0x3A:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            u8 tmp = *loc.dst8;
            u8 tmp1 = *loc.src8;
            u16 tmp2 = tmp - tmp1;
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOSub(tmp1,tmp,false);
            handleP(tmp2&0xFF,false);
            handleZ(tmp2&0xFF);
            handleS(tmp2&0xFF,false);
            ip+=2;
            break;
        }
        case 0x3B:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            u16 tmp = *loc.dst16;
            u16 tmp1 = *loc.src16;
            u32 tmp2 = tmp - tmp1;
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOSub(tmp1,tmp,true);
            handleP(tmp2&0xFFFF,true);
            handleZ(tmp2&0xFFFF);
            handleS(tmp2&0xFFFF,true);
            ip+=2;
            break;
        }
        case 0x3C:
        {
            u8 tmp = axreg.parts.l;
            u8 tmp1 = RAM::rb(cs,ip+1);
            u16 tmp2 = tmp - tmp1;
            handleC(tmp2,false);
            handleA(tmp,tmp1);
            handleOSub(tmp,tmp1,false);
            handleP(tmp2&0xFF,false);
            handleZ(tmp2&0xFF);
            handleS(tmp2&0xFF,false);
            ip+=2;
            break;
        }
        case 0x3D:
        {
            u16 tmp = axreg.w;
            u16 tmp1 = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            u32 tmp2 = tmp - tmp1;
            handleC(tmp2,true);
            handleA(tmp,tmp1);
            handleOSub(tmp,tmp1,true);
            handleP(tmp2&0xFFFF,true);
            handleZ(tmp2&0xFFFF);
            handleS(tmp2&0xFFFF,true);
            ip+=3;
            break;
        }
        case 0x40:
        {
            u16 tmp = axreg.w;
            handleA(tmp,1);
            handleOAdd(tmp,1,true);
            axreg.w++;
            handleP(axreg.w,true);
            handleZ(axreg.w);
            handleS(axreg.w,true);
            ip+=1;
            break;
        }
        case 0x41:
        {
            u16 tmp = cxreg.w;
            handleA(tmp,1);
            handleOAdd(tmp,1,true);
            cxreg.w++;
            handleP(cxreg.w,true);
            handleZ(cxreg.w);
            handleS(cxreg.w,true);
            ip+=1;
            break;
        }
        case 0x42:
        {
            u16 tmp = dxreg.w;
            handleA(tmp,1);
            handleOAdd(tmp,1,true);
            dxreg.w++;
            handleP(dxreg.w,true);
            handleZ(dxreg.w);
            handleS(dxreg.w,true);
            ip+=1;
            break;
        }
        case 0x43:
        {
            u16 tmp = bxreg.w;
            handleA(tmp,1);
            handleOAdd(tmp,1,true);
            bxreg.w++;
            handleP(bxreg.w,true);
            handleZ(bxreg.w);
            handleS(bxreg.w,true);
            ip+=1;
            break;
        }
        case 0x44:
        {
            u16 tmp = sp;
            handleA(tmp,1);
            handleOAdd(tmp,1,true);
            sp++;
            handleP(sp,true);
            handleZ(sp);
            handleS(sp,true);
            ip+=1;
            break;
        }
        case 0x45:
        {
            u16 tmp = bp;
            handleA(tmp,1);
            handleOAdd(tmp,1,true);
            bp++;
            handleP(bp,true);
            handleZ(bp);
            handleS(bp,true);
            ip+=1;
            break;
        }
        case 0x46:
        {
            u16 tmp = si;
            handleA(tmp,1);
            handleOAdd(tmp,1,true);
            si++;
            handleP(si,true);
            handleZ(si);
            handleS(si,true);
            ip+=1;
            break;
        }
        case 0x47:
        {
            u16 tmp = di;
            handleA(tmp,1);
            handleOAdd(tmp,1,true);
            di++;
            handleP(di,true);
            handleZ(di);
            handleS(di,true);
            ip+=1;
            break;
        }
        case 0x48:
        {
            u16 tmp = axreg.w;
            handleA(tmp,1);
            handleOSub(tmp,1,true);
            axreg.w--;
            handleP(axreg.w,true);
            handleZ(axreg.w);
            handleS(axreg.w,true);
            ip+=1;
            break;
        }
        case 0x49:
        {
            u16 tmp = cxreg.w;
            handleA(tmp,1);
            handleOSub(tmp,1,true);
            cxreg.w--;
            handleP(cxreg.w,true);
            handleZ(cxreg.w);
            handleS(cxreg.w,true);
            ip+=1;
            break;
        }
        case 0x4A:
        {
            u16 tmp = dxreg.w;
            handleA(tmp,1);
            handleOSub(tmp,1,true);
            dxreg.w--;
            handleP(dxreg.w,true);
            handleZ(dxreg.w);
            handleS(dxreg.w,true);
            ip+=1;
            break;
        }
        case 0x4B:
        {
            u16 tmp = bxreg.w;
            handleA(tmp,1);
            handleOSub(tmp,1,true);
            bxreg.w--;
            handleP(bxreg.w,true);
            handleZ(bxreg.w);
            handleS(bxreg.w,true);
            ip+=1;
            break;
        }
        case 0x4C:
        {
            u16 tmp = sp;
            handleA(tmp,1);
            handleOSub(tmp,1,true);
            sp--;
            handleP(sp,true);
            handleZ(sp);
            handleS(sp,true);
            ip+=1;
            break;
        }
        case 0x4D:
        {
            u16 tmp = bp;
            handleA(tmp,1);
            handleOSub(tmp,1,true);
            bp--;
            handleP(bp,true);
            handleZ(bp);
            handleS(bp,true);
            ip+=1;
            break;
        }
        case 0x4E:
        {
            u16 tmp = si;
            handleA(tmp,1);
            handleOSub(tmp,1,true);
            si--;
            handleP(si,true);
            handleZ(si);
            handleS(si,true);
            ip+=1;
            break;
        }
        case 0x4F:
        {
            u16 tmp = di;
            handleA(tmp,1);
            handleOSub(tmp,1,true);
            di--;
            handleP(di,true);
            handleZ(di);
            handleS(di,true);
            ip+=1;
            break;
        }
        case 0x70:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            if((flags & 0x0800)) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x71:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            if(!(flags & 0x0800)) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x72:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            if((flags & 0x0001)) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x73:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            if(!(flags & 0x0001)) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x74:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            if((flags & 0x0040)) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x75:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            if(!(flags & 0x0040)) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x76:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            if((flags & 0x0004) || (flags & 0x0001)) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x77:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            if(!(flags & 0x0004) && !(flags & 0x0001)) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x78:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            if((flags & 0x0080)) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x79:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            if(!(flags & 0x0080)) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x7A:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            if((flags & 0x0004)) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x7B:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            if(!(flags & 0x0004)) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x7C:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            bool tmp1 = flags >> 7;
            bool tmp2 = flags >> 11;
            if(tmp1 != tmp2) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x7D:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            bool tmp1 = flags >> 7;
            bool tmp2 = flags >> 11;
            if(tmp1 == tmp2) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x7E:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            bool tmp1 = flags >> 7;
            bool tmp2 = flags >> 11;
            if((tmp1 != tmp2) && (flags & 0x0040)) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x7F:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            bool tmp1 = flags >> 7;
            bool tmp2 = flags >> 11;
            if((tmp1 == tmp2) && !(flags & 0x0040)) ip += (s8)tmp;
            ip+=2;
            break;
        }
        case 0x80:
        case 0x82:
        {
            u8 op2 = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,op2,true,false);
            switch(op2&0x38)
            {
                case 0x00:
                {
                    u8 tmp = *loc.src8;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u16 tmp2 = tmp + tmp1;
                    handleC(tmp2,false);
                    handleA(tmp,tmp1);
                    handleOAdd(tmp,tmp1,false);
                    *loc.src8 += tmp1;
                    handleP(*loc.src8,false);
                    handleZ(*loc.src8);
                    handleS(*loc.src8,false);
                    break;
                }
                case 0x08:
                {
                    u8 tmp = *loc.src8;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u16 tmp2 = tmp | tmp1;
                    handleC(tmp2,false);
                    *loc.src8 |= tmp1;
                    handleP(*loc.src8,false);
                    handleZ(*loc.src8);
                    handleS(*loc.src8,false);
                    break;
                }
                case 0x10:
                {
                    u8 tmp = *loc.src8;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u16 tmp2 = tmp + tmp1 + (flags & 1);
                    handleC(tmp2,false);
                    handleA(tmp,tmp1);
                    handleOAdd(tmp,tmp1,false);
                    *loc.src8 += tmp1 + (flags & 1);
                    handleP(*loc.src8,false);
                    handleZ(*loc.src8);
                    handleS(*loc.src8,false);
                    break;
                }
                case 0x18:
                {
                    u8 tmp = *loc.src8;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u16 tmp2 = tmp - tmp1 - (flags & 1);
                    handleC(tmp2,false);
                    handleA(tmp,tmp1);
                    handleOSub(tmp,tmp1,false);
                    *loc.src8 -= tmp1 + (flags & 1);
                    handleP(*loc.src8,false);
                    handleZ(*loc.src8);
                    handleS(*loc.src8,false);
                    break;
                }
                case 0x20:
                {
                    u8 tmp = *loc.src8;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u16 tmp2 = tmp & tmp1;
                    handleC(tmp2,false);
                    *loc.src8 &= tmp1;
                    handleP(*loc.src8,false);
                    handleZ(*loc.src8);
                    handleS(*loc.src8,false);
                    break;
                }
                case 0x28:
                {
                    u8 tmp = *loc.src8;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u16 tmp2 = tmp - tmp1;
                    handleC(tmp2,false);
                    handleA(tmp,tmp1);
                    handleOSub(tmp,tmp1,false);
                    *loc.src8 -= tmp1;
                    handleP(*loc.src8,false);
                    handleZ(*loc.src8);
                    handleS(*loc.src8,false);
                    break;
                }
                case 0x30:
                {
                    u8 tmp = *loc.src8;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u16 tmp2 = tmp ^ tmp1;
                    handleC(tmp2,false);
                    *loc.src8 ^= tmp1;
                    handleP(*loc.src8,false);
                    handleZ(*loc.src8);
                    handleS(*loc.src8,false);
                    break;
                }
                case 0x38:
                {
                    u8 tmp = *loc.src8;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u16 tmp2 = tmp - tmp1;
                    handleC(tmp2,false);
                    handleA(tmp,tmp1);
                    handleOSub(tmp,tmp1,false);
                    handleP(tmp2&0xFF,false);
                    handleZ(tmp2&0xFF);
                    handleS(tmp2&0xFF,false);
                    break;
                }
            }
            ip+=3;
            break;
        }
        case 0x81:
        {
            u8 op2 = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,op2,true,false);
            switch(op2&0x38)
            {
                case 0x00:
                {
                    u8 tmp = *loc.src16;
                    u16 tmp1 = RAM::rb(cs,ip+2)|(RAM::rb(cs,ip+3)<<8);
                    u32 tmp2 = tmp + tmp1;
                    handleC(tmp2,true);
                    handleA(tmp,tmp1);
                    handleOAdd(tmp,tmp1,true);
                    *loc.src16 += tmp1;
                    handleP(*loc.src16,true);
                    handleZ(*loc.src16);
                    handleS(*loc.src16,true);
                    break;
                }
                case 0x08:
                {
                    u16 tmp = *loc.src16;
                    u16 tmp1 = RAM::rb(cs,ip+2)|(RAM::rb(cs,ip+3)<<8);
                    u16 tmp2 = tmp | tmp1;
                    handleC(tmp2,true);
                    *loc.src16 |= tmp1;
                    handleP(*loc.src16,true);
                    handleZ(*loc.src16);
                    handleS(*loc.src16,true);
                    break;
                }
                case 0x10:
                {
                    u16 tmp = *loc.src16;
                    u16 tmp1 = RAM::rb(cs,ip+2)|(RAM::rb(cs,ip+3)<<8);
                    u32 tmp2 = tmp + tmp1 + (flags & 1);
                    handleC(tmp2,true);
                    handleA(tmp,tmp1);
                    handleOAdd(tmp,tmp1,true);
                    *loc.src16 += tmp1 + (flags & 1);
                    handleP(*loc.src16,true);
                    handleZ(*loc.src16);
                    handleS(*loc.src16,true);
                    break;
                }
                case 0x18:
                {
                    u16 tmp = *loc.src16;
                    u16 tmp1 = RAM::rb(cs,ip+2)|(RAM::rb(cs,ip+3)<<8);
                    u32 tmp2 = tmp - tmp1 - (flags & 1);
                    handleC(tmp2,true);
                    handleA(tmp,tmp1);
                    handleOSub(tmp,tmp1,true);
                    *loc.src16 -= tmp1 + (flags & 1);
                    handleP(*loc.src8,true);
                    handleZ(*loc.src8);
                    handleS(*loc.src8,true);
                    break;
                }
                case 0x20:
                {
                    u16 tmp = *loc.src16;
                    u16 tmp1 = RAM::rb(cs,ip+2)|(RAM::rb(cs,ip+3)<<8);
                    u16 tmp2 = tmp & tmp1;
                    handleC(tmp2,true);
                    *loc.src16 &= tmp1;
                    handleP(*loc.src16,true);
                    handleZ(*loc.src16);
                    handleS(*loc.src16,true);
                    break;
                }
                case 0x28:
                {
                    u16 tmp = *loc.src16;
                    u16 tmp1 = RAM::rb(cs,ip+2)|(RAM::rb(cs,ip+3)<<8);
                    u16 tmp2 = tmp - tmp1;
                    handleC(tmp2,true);
                    handleA(tmp,tmp1);
                    handleOSub(tmp,tmp1,true);
                    *loc.src16 -= tmp1;
                    handleP(*loc.src16,true);
                    handleZ(*loc.src16);
                    handleS(*loc.src16,true);
                    break;
                }
                case 0x30:
                {
                    u16 tmp = *loc.src16;
                    u16 tmp1 = RAM::rb(cs,ip+2)|(RAM::rb(cs,ip+3)<<8);
                    u16 tmp2 = tmp ^ tmp1;
                    handleC(tmp2,true);
                    *loc.src16 ^= tmp1;
                    handleP(*loc.src16,true);
                    handleZ(*loc.src16);
                    handleS(*loc.src16,true);
                    break;
                }
                case 0x38:
                {
                    u16 tmp = *loc.src16;
                    u16 tmp1 = RAM::rb(cs,ip+2)|(RAM::rb(cs,ip+3)<<8);
                    u32 tmp2 = tmp - tmp1;
                    handleC(tmp2,false);
                    handleA(tmp,tmp1);
                    handleOSub(tmp,tmp1,false);
                    handleP(tmp2&0xFFFF,false);
                    handleZ(tmp2&0xFFFF);
                    handleS(tmp2&0xFFFF,false);
                    break;
                }
            }
            ip+=4;
            break;
        }
        case 0x83:
        {
            u8 op2 = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,op2,true,false);
            switch(op2&0x38)
            {
                case 0x00:
                {
                    u8 tmp = *loc.src16;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u32 tmp2 = tmp + tmp1;
                    handleC(tmp2,true);
                    handleA(tmp,tmp1);
                    handleOAdd(tmp,tmp1,true);
                    *loc.src16 += tmp1;
                    handleP(*loc.src16,true);
                    handleZ(*loc.src16);
                    handleS(*loc.src16,true);
                    break;
                }
                case 0x08:
                {
                    u16 tmp = *loc.src16;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u16 tmp2 = tmp | tmp1;
                    handleC(tmp2,true);
                    *loc.src16 |= tmp1;
                    handleP(*loc.src16,true);
                    handleZ(*loc.src16);
                    handleS(*loc.src16,true);
                    break;
                }
                case 0x10:
                {
                    u16 tmp = *loc.src16;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u32 tmp2 = tmp + tmp1 + (flags & 1);
                    handleC(tmp2,true);
                    handleA(tmp,tmp1);
                    handleOAdd(tmp,tmp1,true);
                    *loc.src16 += tmp1 + (flags & 1);
                    handleP(*loc.src16,true);
                    handleZ(*loc.src16);
                    handleS(*loc.src16,true);
                    break;
                }
                case 0x18:
                {
                    u16 tmp = *loc.src16;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u32 tmp2 = tmp - tmp1 - (flags & 1);
                    handleC(tmp2,true);
                    handleA(tmp,tmp1);
                    handleOSub(tmp,tmp1,true);
                    *loc.src16 -= tmp1 + (flags & 1);
                    handleP(*loc.src8,true);
                    handleZ(*loc.src8);
                    handleS(*loc.src8,true);
                    break;
                }
                case 0x20:
                {
                    u16 tmp = *loc.src16;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u16 tmp2 = tmp & tmp1;
                    handleC(tmp2,true);
                    *loc.src16 &= tmp1;
                    handleP(*loc.src16,true);
                    handleZ(*loc.src16);
                    handleS(*loc.src16,true);
                    break;
                }
                case 0x28:
                {
                    u16 tmp = *loc.src16;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u16 tmp2 = tmp - tmp1;
                    handleC(tmp2,true);
                    handleA(tmp,tmp1);
                    handleOSub(tmp,tmp1,true);
                    *loc.src16 -= tmp1;
                    handleP(*loc.src16,true);
                    handleZ(*loc.src16);
                    handleS(*loc.src16,true);
                    break;
                }
                case 0x30:
                {
                    u16 tmp = *loc.src16;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u16 tmp2 = tmp ^ tmp1;
                    handleC(tmp2,true);
                    *loc.src16 ^= tmp1;
                    handleP(*loc.src16,true);
                    handleZ(*loc.src16);
                    handleS(*loc.src16,true);
                    break;
                }
                case 0x38:
                {
                    u16 tmp = *loc.src16;
                    u8 tmp1 = RAM::rb(cs,ip+2);
                    u32 tmp2 = tmp - tmp1;
                    handleC(tmp2,false);
                    handleA(tmp,tmp1);
                    handleOSub(tmp,tmp1,false);
                    handleP(tmp2&0xFFFF,false);
                    handleZ(tmp2&0xFFFF);
                    handleS(tmp2&0xFFFF,false);
                    break;
                }
            }
            ip+=3;
            break;
        }
        case 0x88:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            *loc.src8 = *loc.dst8;
            ip+=2;
            break;
        }
        case 0x89:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            *loc.src16 = *loc.dst16;
            ip+=2;
            break;
        }
        case 0x8A:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,false,false);
            *loc.dst8 = *loc.src8;
            ip+=2;
            break;
        }
        case 0x8B:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            *loc.dst16 = *loc.src16;
            ip+=2;
            break;
        }
        case 0x8C:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,true);
            *loc.src16 = *loc.dst16;
            ip+=2;
            break;
        }
        case 0x8E:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,true);
            *loc.dst16 = *loc.src16;
            ip+=2;
            break;
        }
        case 0xB0:
        {
            axreg.parts.l = RAM::rb(cs,ip+1);
            ip+=2;
            break;
        }
        case 0xB1:
        {
            cxreg.parts.l = RAM::rb(cs,ip+1);
            ip+=2;
            break;
        }
        case 0xB2:
        {
            dxreg.parts.l = RAM::rb(cs,ip+1);
            ip+=2;
            break;
        }
        case 0xB3:
        {
            bxreg.parts.l = RAM::rb(cs,ip+1);
            ip+=2;
            break;
        }
        case 0xB4:
        {
            axreg.parts.h = RAM::rb(cs,ip+1);
            ip+=2;
            break;
        }
        case 0xB5:
        {
            cxreg.parts.h = RAM::rb(cs,ip+1);
            ip+=2;
            break;
        }
        case 0xB6:
        {
            dxreg.parts.h = RAM::rb(cs,ip+1);
            ip+=2;
            break;
        }
        case 0xB7:
        {
            bxreg.parts.h = RAM::rb(cs,ip+1);
            ip+=2;
            break;
        }
        case 0xB8:
        {
            axreg.w = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            ip+=3;
            break;
        }
        case 0xB9:
        {
            cxreg.w = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            ip+=3;
            break;
        }
        case 0xBA:
        {
            dxreg.w = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            ip+=3;
            break;
        }
        case 0xBB:
        {
            bxreg.w = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            ip+=3;
            break;
        }
        case 0xBC:
        {
            sp = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            ip+=3;
            break;
        }
        case 0xBD:
        {
            bp = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            ip+=3;
            break;
        }
        case 0xBE:
        {
            si = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            ip+=3;
            break;
        }
        case 0xBF:
        {
            di = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            ip+=3;
            break;
        }
        case 0xC7:
        {
            u8 modrm = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,modrm,true,false);
            *loc.src16 = RAM::rb(cs,ip+2) | (RAM::rb(cs,ip+3)<<8);
            ip+=4;
            break;
        }
        case 0xD1:
        {
            u8 op2 = RAM::rb(cs,ip+1);
            locs loc = decodeops(seg,op2,true,false);
            switch(op2&0x38)
            {
                case 0x00:
                {
                    u16 tmp = *loc.src16;
                    *loc.src16 = (*loc.src16 << 1) | (*loc.src16 >> 15);
                    if(tmp & 0x8000) flags |= 0x0001;
                    else flags &= 0xFFFE;
                    break;
                }
                case 0x08:
                {
                    u16 tmp = *loc.src16;
                    *loc.src16 = (*loc.src16 >> 1) | (*loc.src16 << 15);
                    if(tmp & 0x0001) flags |= 0x0001;
                    else flags &= 0xFFFE;
                    break;
                }
                case 0x10:
                {
                    u16 tmp = *loc.src16;
                    u16 tmp1 = flags & 1;
                    flags = (flags & 0xFFFE) | (tmp >> 15);
                    *loc.src16 = (*loc.src16 << 1) | tmp1;
                    break;
                }
                case 0x18:
                {
                    u16 tmp = *loc.src16;
                    u16 tmp1 = flags & 1;
                    flags = (flags & 0xFFFE) | (tmp & 1);
                    *loc.src16 = (*loc.src16 >> 1) | tmp1;
                    break;
                }
                case 0x20:
                {
                    u16 tmp = *loc.src16 >> 15;
                    flags = (flags & 0xFFFE) + (tmp & 1);
                    *loc.src16 <<= 1;
                    handleP(*loc.src16,true);
                    handleZ(*loc.src16);
                    handleS(*loc.src16,true);
                    break;
                }
                case 0x28:
                {
                    u16 tmp = *loc.src16;
                    flags = (flags & 0xFFFE) + (tmp & 1);
                    *loc.src16 >>= 1;
                    handleP(*loc.src16,true);
                    handleZ(*loc.src16);
                    handleS(*loc.src16,true);
                    break;
                }
                case 0x38:
                {
                    u16 tmp = *loc.src16;
                    flags = (flags & 0xFFFE) + (tmp & 1);
                    *loc.src16 >>= 1;
                    *loc.src16 |= ((*loc.src16 << 1) & 0x8000);
                    handleP(*loc.src16,true);
                    handleZ(*loc.src16);
                    handleS(*loc.src16,true);
                    break;
                }
            }
            ip+=2;
            break;
        }
        case 0xE6:
        {
            u8 tmp = RAM::rb(cs,ip+1);
            IO::wb(tmp,axreg.parts.l);
            ip+=2;
            break;
        }
        case 0xEA:
        {
            u16 tmp = RAM::rb(cs,ip+1) | (RAM::rb(cs,ip+2)<<8);
            u16 tmp1 = RAM::rb(cs,ip+3) | (RAM::rb(cs,ip+4)<<8);
            cs = tmp1;
            ip = tmp;
            break;
        }
        case 0xEC:
        {
            axreg.parts.l = IO::rb(dxreg.w);
            ip++;
            break;
        }
        case 0xED:
        {
            axreg.w = IO::rw(dxreg.w);
            ip++;
            break;
        }
        case 0xEE:
        {
            IO::wb(dxreg.w,axreg.parts.l);
            ip++;
            break;
        }
        case 0xEF:
        {
            IO::ww(dxreg.w,axreg.w);
            ip++;
            break;
        }
        case 0xF8:
        {
            flags &= 0xFFFE;
            ip++;
            break;
        }
        case 0xF9:
        {
            flags |= 0x0001;
            ip++;
            break;
        }
        case 0xFA:
        {
            flags &= 0xFDFF;
            ip++;
            break;
        }
        case 0xFB:
        {
            flags |= 0x0200;
            ip++;
            break;
        }
        case 0xFC:
        {
            flags &= 0xFBFF;
            ip++;
            break;
        }
        case 0xFD:
        {
            flags |= 0x0400;
            ip++;
            break;
        }
        }
    }

    //For debugging purposes.

    printf("ax=%04X\n",axreg.w);
    printf("bx=%04X\n",bxreg.w);
    printf("cx=%04X\n",cxreg.w);
    printf("dx=%04X\n",dxreg.w);
    printf("cs=%04X\n",cs);
    printf("ip=%04X\n",ip);
    printf("ds=%04X\n",ds);
    printf("es=%04X\n",es);
    printf("ss=%04X\n",ss);
    printf("si=%04X\n",si);
    printf("di=%04X\n",di);
    printf("sp=%04X\n",sp);
    printf("bp=%04X\n",bp);
    printf("flags=%04X\n",flags);
}

void tick()
{
    if(!halted)
    {
        u8 op = RAM::rb(cs,ip);

        switch(op)
        {
        case 0x26:
        {
            ip++;
            seg = SEG_ES;
            printf("ES: ");
            tick();
            break;
        }
        case 0x2E:
        {
            ip++;
            seg = SEG_CS;
            printf("CS: ");
            tick();
            break;
        }
        case 0x36:
        {
            ip++;
            seg = SEG_SS;
            printf("SS: ");
            tick();
            break;
        }
        case 0x3E:
        {
            ip++;
            seg = SEG_DS;
            printf("DS: ");
            tick();
            break;
        }
        case 0xF0:
        {
            ip++;
            printf("LOCK: ");
            tick();
            break;
        }
        case 0xF2:
        {
            ip++;
            rep = REP_NE;
            printf("REPNE: ");
            tick();
            break;
        }
        case 0xF3:
        {
            ip++;
            rep = REP_EQ;
            printf("REPE: ");
            tick();
            break;
        }
        case 0xF4:
        {
            ip++;
            halted = true;
            printf("HLT");
            break;
        }
        default:
        {
            rtick();
            break;
        }
        }
    }
}
}

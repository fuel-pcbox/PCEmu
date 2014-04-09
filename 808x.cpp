#include "808x.h"
#include "ram.h"

namespace CPU808X
{
bool irq;
u8 irqnum;

reg16 axreg, bxreg, cxreg, dxreg;
u16 si,di,bp,sp;
u16 cs,ds,es,ss;
u16 ip;

u16 flags;

bool halted;

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
            res.src16 = &bxreg.wreg.w;
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

inline void handleP(u16 val, bool word)
{
    if(word)
    {
        u8 v = 0;
        for(int i = 0;i < 16;i++)
        {
            if(val & (1 << i)) v ^= 1;
        }
        if(v) flags |= 0x0004;
        else flags &= 0xFFFB;
    }
    else
    {
        u8 v = 0;
        for(int i = 0;i < 8;i++)
        {
            if(val & (1 << i)) v ^= 1;
        }
        if(v) flags |= 0x0004;
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

void rtick()
{
    u8 op = RAM::rb(cs,ip);
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
            handleS(tmp3,false);
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
            handleS(tmp3,true);
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
            handleS(tmp3,false);
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
            handleS(tmp3,true);
            ip+=3;
            break;
        }
		case 0x06:
        {
            sp-=2;
            RAM::wb(ss,sp,cs);
            ip+=1;
            break;
        }
        case 0x0F: //8086/8088 only
        {
            cs = RAM::rb(ss,sp);
            sp+=2;
            ip+=1;
            break;
        }
        }
    }

    log("ax=%04X\n",axreg.w);
    log("bx=%04X\n",bxreg.w);
    log("cx=%04X\n",cxreg.w);
    log("dx=%04X\n",dxreg.w);
    log("cs=%04X\n",cs);
    log("ip=%04X\n",ip);
    log("ds=%04X\n",ds);
    log("es=%04X\n",es);
    log("ss=%04X\n",ss);
    log("si=%04X\n",si);
    log("di=%04X\n",di);
    log("sp=%04X\n",sp);
    log("bp=%04X\n",bp);
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
            log("ES: ");
            tick()
            break;
        }
        case 0x2E:
        {
            ip++;
            seg = SEG_CS;
            log("CS: ");
            tick();
            break;
        }
        case 0x36:
        {
            ip++;
            seg = SEG_SS;
            log("SS: ");
            tick();
            break;
        }
        case 0x3E:
        {
            ip++;
            seg = SEG_DS;
            log("DS: ");
            tick();
            break;
        }
        case 0xF0:
        {
            ip++;
            log("LOCK: ");
            tick();
            break;
        }
        case 0xF2:
        {
            ip++;
            rep = REP_NE;
            log("REPNE: ");
            tick();
            break;
        }
        case 0xF3:
        {
            ip++;
            rep = REP_EQ;
            log("REPE: ");
            tick();
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

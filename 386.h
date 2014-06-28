#ifndef CPU386_H
#define CPU386_H

#include "common.h"

namespace CPU386
{
union reg32
{
    u8 b[2];
    u16 w;
    u32 d;
};

struct sreg
{
    u16 selector;
    u16 flags;
    u32 base;
    u32 limit;
    bool dword;
    bool valid;
};

struct systemtbl
{
    u32 base;
    u16 limit;
};

struct seg_desc
{
    u16 seg;
    u16 flags;
    u32 base;
    u32 limit;
};

enum Reg
{
    EAX, EBX, ECX, EDX, ESI, EDI, EBP, ESP,
    AX, BX, CX, DX, SI, DI, BP, SP,
    AL, AH, BL, BH, CL, CH, DL, DH,
    CS, DS, ES, SS, FS, GS,
    CS_BASE, CS_LIMIT, CS_FLAGS,
    DS_BASE, DS_LIMIT, DS_FLAGS,
    ES_BASE, ES_LIMIT, ES_FLAGS,
    SS_BASE, SS_LIMIT, SS_FLAGS,
    FS_BASE, FS_LIMIT, FS_FLAGS,
    GS_BASE, GS_LIMIT, GS_FLAGS,
    EIP, IP,
    FLAGS, EFLAGS,
    CR0, CR1, CR2, CR3, CR4,
    GDTR_BASE, GDTR_LIMIT,
    IDTR_BASE, IDTR_LIMIT,
    LDTR_BASE, LDTR_LIMIT, LDTR_FLAGS,
    TR, TR_BASE, TR_LIMIT, TR_FLAGS,
    CPL;
};

extern bool irq;
extern u8 irqnum;

extern reg32 eaxreg, ebxreg, ecxreg, edxreg;
extern reg32 esireg,edireg,ebpreg,espreg;
extern sreg cs,ds,es,ss;
extern u32 eip;

extern u32 cr[5];

extern u32 eflags;

extern systemtbl gdtr,idtr;
extern seg_desc ldtr,tr;

extern bool halted;

extern int seg;
extern int rep;

void interrupt(u8 intr);

u32 getaddr(u16 seg, u32 addr);

void init();
void tick();
}

#endif
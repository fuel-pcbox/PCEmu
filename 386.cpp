#include "386.h"

namespace CPU386
{
bool irq;
u8 irqnum;

reg32 eaxreg, ebxreg, ecxreg, edxreg;
reg32 esireg,edireg,ebpreg,espreg;
sreg cs,ds,es,ss;
reg32 eipreg;

u32 cr[5]; 

reg32 eflags;

systemtbl gdtr,idtr;
seg_desc ldtr,tr;

bool halted;

int seg;
int rep;

void push8(u8 val)
{
  sp--;
  RAM::wb(getaddr(ss,sp),val);
}

void push16(u16 val)
{
  sp-=2;
  RAM::wb(getaddr(ss,sp),val&0xFF);
  RAM::wb(getaddr(ss,sp+1),val>>8);
}

void interrupt(u8 intr)
{
  //Check for protected mode
  if(!(cr[0]&1))
  {
    //Real mode
    
    //Push FLAGS, CS, and IP in that order.
    push16(eflags&0xFFFF);
    push16(cs.selector);
    push16(eip&0xFFFF);
    
    //Reload CS and IP.
    cs.selector = RAM::rb(idtr.base + (intr << 2) + 3)|(RAM::rb(idtr.base + (intr << 2) + 4));
    eip = RAM::rb(idtr.base + (intr << 2))|(RAM::rb(idtr.base + (intr << 2) + 1));
    
    //Clear trap flag and interrupt flag.
    flags &= 0xFCFF;
  }
}

u32 getaddr(sreg seg, u32 addr)
{
  //Check for protected mode
  if(!(cr[0]&1))
  {
    //Real mode
    return (seg.selector<<4)+(addr&0xFFFF);
  }
}

void init();
void tick();
}
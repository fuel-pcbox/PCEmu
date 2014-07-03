#ifndef MDA_H
#define MDA_H

namespace MDA
{
extern u8 rom[0x2000];

void init();

void tick();
void update();

u8 rb(u16 addr);
void wb(u16 addr, u8 data);
}

#endif
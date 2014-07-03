#include "ram.h"
#include "mda.h"
#include "interface.h"

namespace MDA
{
u8 rom[0x2000];

u8 crtcindex;
u8 crtc[0x12];

u8 mode;

u8 status;

u8 charcyc;
unsigned y;

u8 screen[720*350*3];

void init()
{
  charcyc = 0;
  y = 0;
  
  for(int i = 0;i<0x12;i++)
  {
    crtc[i] = 0;
  }
}

void tick()
{
  if(charcyc <= crtc[1] && y <= (crtc[6] * (crtc[9]+1)))
  {
  u8 chr = RAM::RAM[0xB0000 + (((crtc[0xC]<<8)|crtc[0xD]) + ((y / 14)*crtc[1]) + charcyc) << 1];
  u8 attr = RAM::RAM[0xB0001 + (((crtc[0xC]<<8)|crtc[0xD]) + ((y / 14)*crtc[1]) + charcyc) << 1];
  
  if(attr != 0x00)
  {
  if((y % 14) & 8)
  {
  screen[(((y*720)+(charcyc*9))*3)+0] = (rom[0x800|(chr*14)+(y % 14)] & 0x80) ? 127 : 0;
  screen[(((y*720)+(charcyc*9))*3)+1] = (rom[0x800|(chr*14)+(y % 14)] & 0x80) ? 127 : 0;
  screen[(((y*720)+(charcyc*9))*3)+2] = (rom[0x800|(chr*14)+(y % 14)] & 0x80) ? 127 : 0;
  
  screen[(((y*720)+(charcyc*9)+1)*3)+0] = (rom[0x800|(chr*14)+(y % 14)] & 0x40) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+1)*3)+1] = (rom[0x800|(chr*14)+(y % 14)] & 0x40) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+1)*3)+2] = (rom[0x800|(chr*14)+(y % 14)] & 0x40) ? 127 : 0;
  
  screen[(((y*720)+(charcyc*9)+2)*3)+0] = (rom[0x800|(chr*14)+(y % 14)] & 0x20) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+2)*3)+1] = (rom[0x800|(chr*14)+(y % 14)] & 0x20) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+2)*3)+2] = (rom[0x800|(chr*14)+(y % 14)] & 0x20) ? 127 : 0;
  
  screen[(((y*720)+(charcyc*9)+3)*3)+0] = (rom[0x800|(chr*14)+(y % 14)] & 0x10) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+3)*3)+1] = (rom[0x800|(chr*14)+(y % 14)] & 0x10) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+3)*3)+2] = (rom[0x800|(chr*14)+(y % 14)] & 0x10) ? 127 : 0;
  
  screen[(((y*720)+(charcyc*9)+4)*3)+0] = (rom[0x800|(chr*14)+(y % 14)] & 0x08) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+4)*3)+1] = (rom[0x800|(chr*14)+(y % 14)] & 0x08) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+4)*3)+2] = (rom[0x800|(chr*14)+(y % 14)] & 0x08) ? 127 : 0;
  
  screen[(((y*720)+(charcyc*9)+5)*3)+0] = (rom[0x800|(chr*14)+(y % 14)] & 0x04) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+5)*3)+1] = (rom[0x800|(chr*14)+(y % 14)] & 0x04) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+5)*3)+2] = (rom[0x800|(chr*14)+(y % 14)] & 0x04) ? 127 : 0;
  
  screen[(((y*720)+(charcyc*9)+6)*3)+0] = (rom[0x800|(chr*14)+(y % 14)] & 0x02) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+6)*3)+1] = (rom[0x800|(chr*14)+(y % 14)] & 0x02) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+6)*3)+2] = (rom[0x800|(chr*14)+(y % 14)] & 0x02) ? 127 : 0;
  
  screen[(((y*720)+(charcyc*9)+7)*3)+0] = (rom[0x800|(chr*14)+(y % 14)] & 0x01) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+7)*3)+1] = (rom[0x800|(chr*14)+(y % 14)] & 0x01) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+7)*3)+2] = (rom[0x800|(chr*14)+(y % 14)] & 0x01) ? 127 : 0;
  }
  else
  {
  screen[(((y*720)+(charcyc*9))*3)+0] = (rom[(chr*14)+(y % 14)] & 0x80) ? 127 : 0;
  screen[(((y*720)+(charcyc*9))*3)+1] = (rom[(chr*14)+(y % 14)] & 0x80) ? 127 : 0;
  screen[(((y*720)+(charcyc*9))*3)+2] = (rom[(chr*14)+(y % 14)] & 0x80) ? 127 : 0;
  
  screen[(((y*720)+(charcyc*9)+1)*3)+0] = (rom[(chr*14)+(y % 14)] & 0x40) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+1)*3)+1] = (rom[(chr*14)+(y % 14)] & 0x40) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+1)*3)+2] = (rom[(chr*14)+(y % 14)] & 0x40) ? 127 : 0;
  
  screen[(((y*720)+(charcyc*9)+2)*3)+0] = (rom[(chr*14)+(y % 14)] & 0x20) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+2)*3)+1] = (rom[(chr*14)+(y % 14)] & 0x20) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+2)*3)+2] = (rom[(chr*14)+(y % 14)] & 0x20) ? 127 : 0;
  
  screen[(((y*720)+(charcyc*9)+3)*3)+0] = (rom[(chr*14)+(y % 14)] & 0x10) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+3)*3)+1] = (rom[(chr*14)+(y % 14)] & 0x10) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+3)*3)+2] = (rom[(chr*14)+(y % 14)] & 0x10) ? 127 : 0;
  
  screen[(((y*720)+(charcyc*9)+4)*3)+0] = (rom[(chr*14)+(y % 14)] & 0x08) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+4)*3)+1] = (rom[(chr*14)+(y % 14)] & 0x08) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+4)*3)+2] = (rom[(chr*14)+(y % 14)] & 0x08) ? 127 : 0;
  
  screen[(((y*720)+(charcyc*9)+5)*3)+0] = (rom[(chr*14)+(y % 14)] & 0x04) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+5)*3)+1] = (rom[(chr*14)+(y % 14)] & 0x04) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+5)*3)+2] = (rom[(chr*14)+(y % 14)] & 0x04) ? 127 : 0;
  
  screen[(((y*720)+(charcyc*9)+6)*3)+0] = (rom[(chr*14)+(y % 14)] & 0x02) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+6)*3)+1] = (rom[(chr*14)+(y % 14)] & 0x02) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+6)*3)+2] = (rom[(chr*14)+(y % 14)] & 0x02) ? 127 : 0;
  
  screen[(((y*720)+(charcyc*9)+7)*3)+0] = (rom[(chr*14)+(y % 14)] & 0x01) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+7)*3)+1] = (rom[(chr*14)+(y % 14)] & 0x01) ? 127 : 0;
  screen[(((y*720)+(charcyc*9)+7)*3)+2] = (rom[(chr*14)+(y % 14)] & 0x01) ? 127 : 0;
  }
  }
  }
  
  charcyc++;
  if(charcyc >= (crtc[0]+1))
  {
    charcyc = 0;
    y++;
    if(y >= ((crtc[4]+crtc[5])*(crtc[9]+1)))
    {
      y = 0;
      update();
    }
  }
}
void update()
{
  u8* p = (u8*)INTERFACE::screen->pixels;
  
  for(int i = 0;i<720*350*3;i++)
  {
    p[i] = screen[i];
  }
  
  INTERFACE::update();
}

u8 rb(u16 addr)
{
  switch(addr)
  {
  case 0x3BA:
  {
    return status;
    break;
  }
  }
  return 0xFF;
}
void wb(u16 addr, u8 data)
{
  switch(addr)
  {
  case 0x3B4:
  {
    crtcindex = data;
    break;
  }
  case 0x3B5:
  {
    crtc[crtcindex] = data;
    break;
  }
  case 0x3B8:
  {
    mode = data;
    break;
  }
  }
}
}

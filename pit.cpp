#include "pit.h"
#include "pic.h"
#include "kbdc.h"

namespace PIT
{

struct pitchan
{
  u16 value;
  u16 latch;
  u16 reload;
  bool gate_in;
  bool gate_out;
  u8 mode;
  u8 accessmode;
  bool flipflop;
  bool bcd;
  int phase;
};

pitchan pits[3];

void init()
{
  for(int i = 0;i<3;i++)
  {
    pits[i].phase = 0;
    pits[i].flipflop = false;
  }
}

void dectimer(int i)
{
  if(!(pits[i].mode & 1))
  {
    pits[i].value--;
    return;
  }
  
  pits[i].value--;
  if((pits[i].value & 0x000F) > 0x0009) pits[i].value -= 0x0006;
  if((pits[i].value & 0x00F0) > 0x0090) pits[i].value -= 0x0060;
  if((pits[i].value & 0x0F00) > 0x0900) pits[i].value -= 0x0600;
  if((pits[i].value & 0xF000) > 0x9000) pits[i].value -= 0x6000;
}

void set_output(int i, bool val)
{
  pits[i].gate_out = val;
  if(val == true && i == 0)
  {
    CPU808X::irq = true;
    CPU808X::irqnum = PIC::pics[0].offset;
  }
  if(val == false && i == 1)
  {
    KBDC::portb ^= 0x10;
  }
  if(i == 2)
  {
    KBDC::portb &= 0xFD;
    KBDC::portb |= pits[2].gate_out << 1;
  }
}

void tick()
{
  for(int i = 0;i<3;i++)
  {
      switch(pits[i].mode)
      {
      case 0:
      {
        if(pits[i].phase == 1)
        {
          pits[i].value = pits[i].reload;
          pits[i].phase = 2;
        }
        else
        {
          if(pits[i].phase == 2 && pits[i].value == 1)
          {
            set_output(i,true);
            pits[i].phase = 3;
            pits[i].value = 0;
          }
          
          dectimer(i);
        }
        break;
      }
      case 2:
      {
        if(pits[i].phase == 1)
        {
          pits[i].value = pits[i].reload;
          pits[i].phase = 2;
        }
        else
        {
          if(pits[i].phase == 3)
          {
            set_output(i,true);
            pits[i].value = pits[i].reload;
            pits[i].phase = 2;
          }
          else if(pits[i].phase && pits[i].value == 1)
          {
            set_output(i,false);
            pits[i].phase = 3;
          }
          dectimer(i);
        }
        break;
      }
      case 3:
      {
        if(pits[i].phase == 1)
        {
          pits[i].value = pits[i].reload;
          pits[i].phase = 2;
        }
        else
        {
          if(pits[i].phase == 2 && pits[i].value == 0)
          {
            set_output(i,false);
            pits[i].value = pits[i].reload;
            pits[i].phase = 3;
          }
          else if(pits[i].phase == 3 && pits[i].value == 0)
          {
            set_output(i,true);
            pits[i].value = pits[i].reload;
            pits[i].phase = 2;
          }
          dectimer(i);
          dectimer(i);
        }
        break;
      }
      }
  }
}

u8 rb(u16 addr)
{
}

void wb(u16 addr, u8 data)
{
  switch(addr)
  {
  case 0x40:
  case 0x41:
  case 0x42:
  {
    int i = addr & 3;
    switch(pits[i].accessmode)
    {
    case 1:
    {
      pits[i].reload = (pits[i].reload & 0xFF00) | data;
    }
    case 2:
    {
      pits[i].reload = (pits[i].reload & 0x00FF) | (data << 8);
    }
    case 3:
    {
      if(pits[i].flipflop) pits[i].reload = (pits[i].reload & 0x00FF) | (data << 8);
      else pits[i].reload = (pits[i].reload & 0xFF00) | data;
      pits[i].flipflop = !pits[i].flipflop;
    }
    }
    break;
  }
  case 0x43:
  {
    int i = (data >> 6) & 3;
    pits[i].accessmode = (data >> 4) & 3;
    pits[i].mode = (data >> 1) & 7;
    pits[i].bcd = data & 1;
    pits[i].phase = 0;
    switch(pits[i].mode)
    {
    case 0:
    {
      set_output(i,false);
      break;
    }
    default:
    {
      set_output(i,true);
      break;
    }
    }
    break;
  }
  }
}

}
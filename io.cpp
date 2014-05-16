#include "io.h"

namespace IO
{
u8 rb(u16 addr)
{
  return IO8::rb(addr);
}

u16 rw(u16 addr)
{
  return IO8::rb(addr) | (IO8::rb(addr+1)<<8);
}

void wb(u16 addr, u8 data)
{
  IO8::wb(addr,data);
}

void ww(u16 addr, u16 data)
{
  IO8::wb(addr,data&0xFF);
  IO8::wb(addr+1,data>>8);
}

}
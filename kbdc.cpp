#include "kbdc.h"

namespace KBDC
{
u8 portb;

void init()
{
}

u8 rb(u16 addr)
{
  switch(addr)
  {
  case 1:
  {
    return portb | 0xCC; //Act like we did the parity and channel checks.
  }
  }
}

void wb(u16 addr, u8 data)
{
}

}
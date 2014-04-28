#include "808x.h"

int main()
{
  printf("PCEmu v1-prealpha\n");
  CPU808X::init();
  CPU808X::tick();
}
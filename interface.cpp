#include "interface.h"

namespace INTERFACE
{
SDL_Surface* screen;

void init()
{
  SDL_Init(SDL_INIT_EVERYTHING);
  screen = SDL_SetVideoMode(720,350,24,SDL_SWSURFACE);
}

void update()
{
  SDL_Flip(screen);
}
}
#ifndef INTERFACE_H
#define INTERFACE_H

#include <SDL/SDL.h>

namespace INTERFACE
{
extern SDL_Surface* screen;

void init();

void update();
}

#endif
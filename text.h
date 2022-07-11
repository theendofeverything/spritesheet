#ifndef __TEXT_H__
#define __TEXT_H__

#include <SDL.h>
typedef struct
{
    char *text;
    SDL_Texture *tex;
    SDL_Rect rect;
    SDL_Color fg;
    int margin;
} TextBox;

#endif // __TEXT_H__


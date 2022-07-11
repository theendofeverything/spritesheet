#ifndef __TEXT_H__
#define __TEXT_H__

#include <SDL.h>
typedef struct
{
    char *text;
    SDL_Texture *tex;
    SDL_Rect fg_rect;
    SDL_Color fg;
    SDL_Rect bg_rect;
    SDL_Color bg;
    int margin;
} TextBox;

#endif // __TEXT_H__


#ifndef __BGND_H__
#define __BGND_H__
#include <SDL.h>

void bgnd_gradient(SDL_Texture **bgnd_tex, SDL_Renderer *ren, WindowInfo wI)
    { // Paint a beautiful background gradient
        //                                    flags, w,  h, bit-depth, masks
        SDL_Surface *surf = SDL_CreateRGBSurface(0, wI.w, wI.h, 32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000);
        // Fill this surface with a beautiful complex gradient
        uint32_t vstart = 2*wI.h; uint32_t hstart = 3*wI.w;    // initial gradient brightness
        uint32_t *p = surf->pixels;
        for( int row=0; row < surf->h; row++ )                    // Walk rows
        {
            for( int col=0; col < surf->w; col++ )                // Walk columns
            {
                uint32_t r, g, b, c32;
                { // Calculate the color
                    r = ((row+vstart)*255/(wI.h+vstart));
                    g = ((col+hstart)*200/(wI.w+hstart));
                    b = ((col+hstart)*255/(wI.w+hstart));
                    c32 = (0xFF << 24) | (r << 16) | (g << 8) | b;
                }
                *p++ = c32;
            }
        }
        *bgnd_tex = SDL_CreateTextureFromSurface(ren, surf);
        SDL_SetTextureBlendMode(*bgnd_tex, SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(surf);
    }

#endif // __BGND_H__

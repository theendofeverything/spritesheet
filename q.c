#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
/* #include "text.h" */
#include "window_info.h"

int main(int argc, char *argv[])
{
    for(int i=0; i<argc; i++) puts(argv[i]);

    // Setup
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Font *font;                                             // Debug overlay font
    { // Setup font
        if(  TTF_Init() <0  )
        { // Error handling: Cannot initialize SDL_ttf
            puts("Cannot initialize SDL_ttf");
            SDL_Quit();
            return EXIT_FAILURE;
        }
        const char *font_file = "fonts/ProggyClean.ttf";
        font = TTF_OpenFont(font_file, 16);
        if(  font == NULL  )
        { // Error handling: font file does not exist
            printf("Cannot open font file. Please check \"%s\" exists.", font_file);
            TTF_Quit();
            SDL_Quit();
            return EXIT_FAILURE;
        }
    }
    WindowInfo wI; WindowInfo_setup(&wI, argc, argv);           // Size and locate window
    SDL_Window *win = SDL_CreateWindow(argv[0], wI.x, wI.y, wI.w, wI.h, wI.flags);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, 0);
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);       // Draw with alpha

    // Game state
    bool quit = false;
    bool show_debug = false;
    while(  quit == false  )
    {
        // UI
        { // Polled
            SDL_Event e;
            while(  SDL_PollEvent(&e)  )
            {
                if(  e.type == SDL_KEYDOWN  )
                {
                    switch(e.key.keysym.sym)
                    {
                        case SDLK_TAB:
                            show_debug = show_debug ? false : true;
                            break;
                        default: break;
                    }
                }
            }
        }
        { // Filtered
            SDL_PumpEvents();                                   // Update event queue
            const Uint8 *k = SDL_GetKeyboardState(NULL);        // Get all keys
            if(  k[SDL_SCANCODE_Q]  ) quit = true;              // q to quit
        }
        // Render
        { // Set dark green background
            SDL_Color bg = {.r=0x1F, .g=0x1F, .b=0x08, .a=0xFF};    // background color
            SDL_SetRenderDrawColor(ren, bg.r, bg.g, bg.b, bg.a);
            SDL_RenderClear(ren);
        }
        { // Present to screen
            SDL_RenderPresent(ren);
            SDL_Delay(10);
        }
    }

    // Shutdown
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    return EXIT_SUCCESS;
}

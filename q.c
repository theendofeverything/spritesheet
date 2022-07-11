#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "text.h"

int main(int argc, char *argv[])
{
    for(int i=0; i<argc; i++) puts(argv[i]);

    // Setup
    SDL_Init(SDL_INIT_VIDEO);
    if(  TTF_Init() <0  )
    {
        puts("Cannot initialize SDL_ttf");
        SDL_Quit();
        return EXIT_FAILURE;
    }
    const char *font_file = "fonts/ProggyClean.ttf";
    TTF_Font *font = TTF_OpenFont(font_file, 16);
    if(  font == NULL  )
    {
        printf("Cannot open font file. Please check \"%s\" exists.", font_file);
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Shutdown
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    return EXIT_SUCCESS;
}

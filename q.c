/* *************TODO***************
 * Walk spritesheet while it is still a surface.
 * Split the spritesheet into animation frames.
 * Write each frame to a texture.
 * *******************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "text.h"
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

    // Turn spritesheet into sprite animation
    const char *png_path = "art/Kerbey-blinkey-tiredey.png";
    SDL_Surface *img_surf = IMG_Load(png_path);
    if(  img_surf == NULL  )
    {
        printf("Failed to load \"%s\": %s", png_path, IMG_GetError());
        SDL_DestroyWindow(win); SDL_DestroyRenderer(ren);
        TTF_CloseFont(font); TTF_Quit(); SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_Texture *img_tex = SDL_CreateTextureFromSurface(ren, img_surf);
    SDL_FreeSurface(img_surf);

    // Game state
    bool quit = false;
    bool show_debug = false;
    TextBox tb;                                                 // Debug overlay text box
    char text_buffer[1024];                                     // Max 1024 characters
    { // Set up the text box
        tb.margin = 5;                                          // Margin relative to window
        tb.fg = (SDL_Color){255,255,255,255};                   // Text color: white
        tb.fg_rect=(SDL_Rect){0};                               // Init size to 0
        tb.fg_rect.x = tb.margin;                               // Left edge of text
        tb.fg_rect.y = tb.margin;                               // Top edge of text
        tb.bg = (SDL_Color){0,0,0,127};                         // Bgnd: Black 50% opacity
        tb.bg_rect=(SDL_Rect){0};                               // Init bgnd size
        tb.bg_rect.w = wI.w;                                    // Bgnd is full window width
        tb.text = text_buffer;                                  // Point at text buffer
    }
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
        { // Draw the spritesheet
            SDL_RenderCopy(ren, img_tex, NULL, NULL);
        }
        if(show_debug)
        { // Debug overlay
            { // Put text in the text box
                char *d = tb.text;                              // d : walk dst
                { const char *str = "Example: ";                // Copy this text
                    const char *c = str;                        // c : walk src
                    while(*c!='\0'){*d++=*c++;} *d='\0';        // Copy char by char
                }
                SDL_Surface *surf = TTF_RenderText_Blended_Wrapped(font, tb.text, tb.fg,
                                                wI.w-tb.margin);   // Wrap text here
                tb.tex = SDL_CreateTextureFromSurface(ren, surf);
                SDL_FreeSurface(surf);
                SDL_QueryTexture(tb.tex, NULL, NULL, &tb.fg_rect.w, &tb.fg_rect.h);
            }
            { // Draw text
                tb.bg_rect.h = tb.fg_rect.h + 2*tb.margin;
                SDL_SetRenderDrawColor(ren, tb.bg.r, tb.bg.g, tb.bg.b, tb.bg.a);
                // Render bgnd
                SDL_RenderFillRect(ren, &tb.bg_rect);
                // Render text
                SDL_RenderCopy(ren, tb.tex, NULL, &tb.fg_rect);
                SDL_DestroyTexture(tb.tex);
            }
        }
        { // Present to screen
            SDL_RenderPresent(ren);
            SDL_Delay(10);
        }
    }

    // Shutdown
    SDL_DestroyTexture(img_tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    return EXIT_SUCCESS;
}

/* *************Sprite Sheet: Overview***************
 * - load sprite sheet png as SDL surface img_surf
 * - load surface into SDL texture img_tex
 *   (img_tex has all frames)
 * - Select frame to render WHEN COPYING THE TEXTURE TO THE RENDERER:
 *
 *   Example:
 *   SDL_RenderCopy(ren, img_tex, &sprite_frame, &sprite_render);
 *
 *   Rects:
 *   sprite_frame : SDL_Rect identify one frame on the sprite sheet
 *   sprite_render : SDL_Rect defining size and location of rendered
 *   frame on screen
 * *******************************/
/* *************Sprite Sheet: Select Frames***************
 * - Every frame has size sprite_size x sprite_size
 * - Example: rect selects the first frame (x=0, y=0)
 *
 *      SDL_Rect sprite_frame = {.x=0, .y=0, .w=sprite_size, .h=sprite_size};
 *
 * - Define the size and location of the sprite frame on the screen.
 * - Example: rect centers the sprite on the screen and renders it to scale
 *
 *      SDL_Rect sprite_render = { .x=wI.w-sprite_size}/2,
 *                                 .y=wI.h-sprite_size}/2,
 *                                 .w=sprite_size,
 *                                 .h=sprite_size }
 *
 * *******************************/
/* *************TODO***************
 * Load all sprite sheets.
 * Add keyboard control to move sprite.
 * *******************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "text.h"
#include "window_info.h"

void next_frame(SDL_Rect *frame, bool *run, int sprite_size, int nframes)
{ // Load next frame rect. Set run to false when loading the last frame rect.
    /* *************DOC***************
     * Loads rect into frame.
     * The rect selects the next frame to render from the sprite sheet texture.
     *
     * Determine the next frame by looking at the x,y of the current frame.
     * If this is the last frame, load run with false.
     * Use nframes to determine if this is the last frame.
     * *******************************/
    // Go to the next frame on the sprite sheet texture
    if(  frame->x >= 7*sprite_size  )                           // 8 frames per row
    { // If last frame on this row, go to next row
        frame->x = 0;
        frame->y += sprite_size;
    }
    else
    { // Go to next column
        frame->x += sprite_size;
    }
    // Stop animation at the last frame
    int max_y = (int)nframes/8;
    int max_x = (nframes%8)-1;
    if(  (frame->x >=max_x*sprite_size) && (frame->y >=max_y*sprite_size)  )
    {
        /* frame->x = 0; frame->y = 0; // Reset animation */
        *run = false;
    }
}

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
    IMG_Init(IMG_INIT_PNG);
    SDL_Texture *img_tex;                                       // One texture for entire sprite sheet
    const int sprite_size=64;                                   // Sprite frames are 64x64
    int sprite_scale = 4;                                       // Initial scale is 4x actual size
    SDL_Rect sprite_render = {.x=(wI.w-sprite_scale*sprite_size)/2, // Initial x-pos: center
                              .y=(wI.h-sprite_scale*sprite_size)/2, // Initial y-pos: center
                              .w=sprite_scale*sprite_size,      // Scale sprite up by 4x
                              .h=sprite_scale*sprite_size       // Scale sprite up by 4x
                              };
    SDL_Rect sprite_frame = {  .x=0, .y=0,                             // start at first frame
                        .w=sprite_size, .h=sprite_size          // 64x64 sprite
                        };
    const char *img_path = "art/Kerbey-blinkey-tiredey.png";
    {
        SDL_Surface *img_surf = IMG_Load(img_path);
        if(  img_surf == NULL  )
        {
            printf("Failed to load \"%s\": %s", img_path, IMG_GetError());
            SDL_DestroyWindow(win); SDL_DestroyRenderer(ren);
            TTF_CloseFont(font); TTF_Quit(); SDL_Quit();
            return EXIT_FAILURE;
        }
        img_tex = SDL_CreateTextureFromSurface(ren, img_surf);
        SDL_FreeSurface(img_surf);
    }

    // Game state
    bool quit = false;
    bool show_debug = false;
    bool run_animation = false;
    int ticks = 0;                                              // Count SDL ticks
    const int ticks_per_anim_frame = 7;                         // Animation frame time
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
                        case SDLK_SPACE:
                            run_animation = true;
                            sprite_frame.x = -1*sprite_size; sprite_frame.y = 0;    // Reset animation
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
            if(  k[SDL_SCANCODE_UP]  )
            {
                sprite_scale++;
                if(  sprite_scale>32  ) sprite_scale=32;
                sprite_render.x=(wI.w-sprite_scale*sprite_size)/2;
                sprite_render.y=(wI.h-sprite_scale*sprite_size)/2;
                sprite_render.w=sprite_scale*sprite_size;
                sprite_render.h=sprite_scale*sprite_size;
            }
            if(  k[SDL_SCANCODE_DOWN]  )
            {
                sprite_scale--;
                if(  sprite_scale<1  ) sprite_scale=1;
                sprite_render.x=(wI.w-sprite_scale*sprite_size)/2;
                sprite_render.y=(wI.h-sprite_scale*sprite_size)/2;
                sprite_render.w=sprite_scale*sprite_size;
                sprite_render.h=sprite_scale*sprite_size;
            }
        }

        if(run_animation)
        {
            if(  ticks < ticks_per_anim_frame  ) ticks++;
            else
            {
                next_frame(&sprite_frame, &run_animation, sprite_size, 30); // 30 frames
                ticks = 0;
            }
        }

        // Render
        { // Set dark green background
            SDL_Color bg = {.r=0x1F, .g=0x1F, .b=0x08, .a=0xFF};    // background color
            SDL_SetRenderDrawColor(ren, bg.r, bg.g, bg.b, bg.a);
            SDL_RenderClear(ren);
        }
        { // Draw the spritesheet
            /* SDL_RenderCopy(ren, img_tex, NULL, NULL); */

            /* SDL_Rect frame1 = {.x=1*sprite_size, .y=0*sprite_size, .w=sprite_size, .h=sprite_size}; */
            SDL_RenderCopy(ren, img_tex, &sprite_frame, &sprite_render);
            /* SDL_RenderCopy(ren, img_tex, &frame1, &sprite_render); */
        }
        if(show_debug)
        { // Debug overlay
            { // Put text in the text box
                char *d = tb.text;                              // d : walk dst
                { const char *str = "Spritesheet: ";                // Copy this text
                    const char *c = str;                        // c : walk src
                    while(*c!='\0'){*d++=*c++;} *d='\0';        // Copy char by char
                }
                { const char *str = img_path;                   // Copy this text
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
    IMG_Quit();
    SDL_DestroyTexture(img_tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    return EXIT_SUCCESS;
}

/* *************Sprite Sheet: Overview***************
 * - load sprite sheet png as SDL surface sprite_surf
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
 * 1. Figure out how to export from Pixaki with transparent background.
 * 2. Load all sprite sheets.
 * 3. Add keyboard control to move sprite.
 * *******************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "text.h"
#include "window_info.h"
#include "bgnd.h"
#include "print.h"
#include "anim.h"
#include "font.h"
#include "sprite.h"

void shutdown(TTF_Font *debug_font,
              SDL_Renderer *ren,
              SDL_Window *win,
              SDL_Texture *bgnd_tex,
              SDL_Texture *img_tex)
{
    IMG_Quit();
    SDL_DestroyTexture(bgnd_tex);
    SDL_DestroyTexture(img_tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_CloseFont(debug_font);
    TTF_Quit();
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    for(int i=0; i<argc; i++) puts(argv[i]);

    // Setup
    SDL_Init(SDL_INIT_VIDEO);                                           // Init SDL

    // Setup debug overlay font
    TTF_Font *debug_font;                                               // Debug overlay font
    { // Setup font
        if(  font_init() < 0  )                                         // Init SDL_ttf
        {
            shutdown(NULL, NULL, NULL, NULL, NULL);
            return EXIT_FAILURE;
        }
        if(  font_load(&debug_font, "fonts/ProggyClean.ttf", 16) < 0  ) // Load font
        {
            shutdown(NULL, NULL, NULL, NULL, NULL);
            return EXIT_FAILURE;
        }
    }

    // Create a Window and a Renderer
    WindowInfo wI; WindowInfo_setup(&wI, argc, argv);                   // Size and locate window
    SDL_Window *win = SDL_CreateWindow(argv[0], wI.x, wI.y, wI.w, wI.h, wI.flags);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, 0);
    if(  SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND) < 0  )    // Draw with alpha
    {
        puts("Cannot draw with alpha channel");
        shutdown(debug_font, ren, win, NULL, NULL); return EXIT_FAILURE;
    }

    // Load the spritesheet
    IMG_Init(IMG_INIT_PNG);                                     // Spritesheet is a PNG
    const char *sprite_path = "art/numbers.png";
    int sprite_size; int sprite_framecnt = 0;
    int sprite_framenum = 1;
    { // Load spritesheet into a Surface to check transparency, size, and number of frames
        SDL_Surface *sprite_surf = IMG_Load(sprite_path);
        if(  sprite_surf == NULL  )
        { // Unable to load image
            printf("Failed to load \"%s\": %s", sprite_path, IMG_GetError());
            shutdown(debug_font, ren, win, NULL, NULL); return EXIT_FAILURE;
        }
        if(  sprite_sheet_has_transparency(sprite_surf, sprite_path) == false )
        {
            SDL_FreeSurface(sprite_surf);
            shutdown(debug_font, ren, win, NULL, NULL);
            return EXIT_FAILURE;
        }
        sprite_size = sprite_get_size(sprite_surf);
        printf("Sprite size: %d\n", sprite_size);fflush(stdout);
        sprite_framecnt = sprite_get_num_frames(sprite_surf, sprite_size);
        printf("OK");fflush(stdout);
        SDL_FreeSurface(sprite_surf);
    }
    SDL_Texture *img_tex;                                       // One texture for entire sprite sheet
    { // Load Texture directly from spritesheet file
        img_tex = IMG_LoadTexture(ren, sprite_path);
        if(  img_tex == NULL  )
        {
            printf("Failed to load \"%s\": %s", sprite_path, IMG_GetError());
            SDL_DestroyWindow(win); SDL_DestroyRenderer(ren);
            TTF_CloseFont(debug_font); TTF_Quit(); SDL_Quit();
            return EXIT_FAILURE;
        }
    }

    // Create a background texture with a sky-colored gradient
    SDL_Texture *bgnd_tex;
    bgnd_gradient(&bgnd_tex, ren, wI);

    int sprite_scale = 2;                                       // Initial scale is 1x actual size
    SDL_Rect sprite_render = {.x=(wI.w-sprite_scale*sprite_size)/2, // Initial x-pos: center
                              .y=(wI.h-sprite_scale*sprite_size)/2, // Initial y-pos: center
                              .w=sprite_scale*sprite_size,      // Scale sprite up by 4x
                              .h=sprite_scale*sprite_size       // Scale sprite up by 4x
                              };
    SDL_Rect sprite_frame = {  .x=0, .y=0,                             // start at first frame
                        .w=sprite_size, .h=sprite_size          // 64x64 sprite
                        };

    // Game state
    bool quit = false;
    bool show_debug = true;
    int run_animation = 0;
    bool run_once = false;
    bool idle_animation = false;
    bool stop_idle_animation = false;
    bool start_penguin_huff = false;
    int delay_btwn_huffs = 0;
    int ticks = 0;                                              // Count SDL ticks
    int ticks_per_anim_frame = 4;                               // Animation frame time
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
        SDL_Keymod kmod = SDL_GetModState();                    // kmod : OR'd modifiers
        { // Filtered
            SDL_PumpEvents();                                   // Update event queue
            const Uint8 *k = SDL_GetKeyboardState(NULL);        // Get all keys
            if(  k[SDL_SCANCODE_ESCAPE]  ) quit = true;         // Esc to quit
            if(0)
            { // Up/Down to zoom in/out
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
        }
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
                            run_once = true;
                            break;
                        case SDLK_RIGHT:
                            stop_idle_animation = true;
                            if(  kmod & (KMOD_LSHIFT|KMOD_RSHIFT)  )
                            { // DEBUG
                                anim_next_frame(&sprite_framenum, sprite_framecnt);
                                anim_load_frame(&sprite_frame, sprite_size, sprite_framenum);
                            }
                            break;
                        case SDLK_LEFT:
                            stop_idle_animation = true;
                            if(  kmod & (KMOD_LSHIFT|KMOD_RSHIFT)  )
                            { // DEBUG
                                anim_prev_frame(&sprite_framenum, sprite_framecnt);
                                anim_load_frame(&sprite_frame, sprite_size, sprite_framenum);
                            }
                            break;
                        default: break;
                    }
                }
                if(  e.type == SDL_KEYUP  )
                {
                    switch(e.key.keysym.sym)
                    {
                        case SDLK_RIGHT:
                        case SDLK_LEFT:
                            if(  (kmod & (KMOD_LSHIFT|KMOD_RSHIFT)) == 0 )
                            {
                                idle_animation = true;
                                stop_idle_animation = false;
                                start_penguin_huff = true;
                                delay_btwn_huffs = sprite_framecnt;
                                ticks_per_anim_frame = 2;           // Initial animation speed
                                sprite_framenum = 1;                   // Reset idle animation
                            }
                            break;
                    }
                }
            }
        }

        if(  run_once  )
        {
            run_once = false;
            run_animation = sprite_framecnt;                       // Run animation once
            ticks_per_anim_frame = 6;
            sprite_framenum = 1;                   // Reset idle animation
            idle_animation = true;
        }
        if(  idle_animation  )
        {
            if(  start_penguin_huff  )
            {
                run_animation = 2*sprite_framecnt;     // Run animation twice
                if (ticks_per_anim_frame > 4) ticks_per_anim_frame = 4; // Clamp at 8
                start_penguin_huff = false;
            }
            if(  run_animation > 0  )
            {
                if(  ticks < ticks_per_anim_frame  ) ticks++;
                else
                {
                    /* OLD_anim_next_frame(&sprite_frame, &run_animation, sprite_size, sprite_framecnt); // 30 frames */
                    anim_next_frame(&sprite_framenum, sprite_framecnt);
                    anim_load_frame(&sprite_frame, sprite_size, sprite_framenum);
                    ticks = 0;
                    run_animation--;                                // Count down num times to run animation
                    if(  (run_animation%sprite_framecnt) == 0  )
                    {
                        ticks_per_anim_frame++;                     // Slow down animation to give is some variation
                    }
                }
            }
            else
            {
                if(  stop_idle_animation  )
                {
                    stop_idle_animation = false;
                    idle_animation = false;
                }
                ticks++;
                if( ticks > delay_btwn_huffs )
                {
                    ticks = 0;
                    start_penguin_huff = true;
                    delay_btwn_huffs += sprite_framecnt;
                }
            }
        }

        // Render
        { // Paint over old video frame with a beautiful background gradient
            SDL_RenderCopy(ren, bgnd_tex, NULL, NULL);
        }
        { // Draw the sprite
            /* SDL_RenderCopy(ren, img_tex, NULL, NULL);           // Draw entire spritesheet */

            SDL_RenderCopy(ren, img_tex, &sprite_frame, &sprite_render); // Draw one frame
        }
        if(show_debug)
        { // Debug overlay
            { // Put text in the text box
                char *d = tb.text;                              // d : see macro "print"
                print("Spritesheet: "); print(sprite_path);
                print(" | ");
                print("Sprite size: "); printint(4, sprite_size); print("x"); printint(4, sprite_size);
                print(" | ");
                print("Animation frame: "); printint(3, sprite_framenum); print(" / "); printint(3, sprite_framecnt);
                print(" | ");
                print("Window size: "); printint(5, wI.w); print("x"); printint(5, wI.h); print(" (wxh)");
                SDL_Surface *surf = TTF_RenderText_Blended_Wrapped(debug_font, tb.text, tb.fg,
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

    shutdown(debug_font, ren, win, bgnd_tex, img_tex);
    return EXIT_SUCCESS;
}

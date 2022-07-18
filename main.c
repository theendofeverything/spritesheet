/* *************Sprite Sheet: Overview***************
 * - Load sprite sheet png as SDL texture tex_PI
 *   (tex_PI has all frames)
 * - Animate by moving the frame rectangle around the spritesheet texture
 * - Copy rectangular section of texture to the renderer
 * - Renderer rectangle sets the size and location on the screen
 *
 *   Example:
 *   SDL_RenderCopy(ren, tex_PI, &sprite_PI->frame, &sprite_PI->render);
 *
 *   Rects:
 *   sprite_PI->frame : SDL_Rect identify one frame on the sprite sheet
 *   sprite_PI->render : SDL_Rect defining size and location of rendered
 *   frame on screen
 * *******************************/
/* *************Sprite Sheet: Select Frames***************
 * - Every frame has size sprite_PI->size x sprite_PI->size
 * - Example: rect selects the first frame (x=0, y=0)
 *
 *      SDL_Rect sprite_PI->frame = {.x=0, .y=0, .w=sprite_PI->size, .h=sprite_PI->size};
 *
 * - Define the size and location of the sprite frame on the screen.
 * - Example: rect centers the sprite on the screen and renders it to scale
 *
 *      SDL_Rect sprite_PI->render = { .x=wI.w-sprite_PI->size}/2,
 *                                 .y=wI.h-sprite_PI->size}/2,
 *                                 .w=sprite_PI->size,
 *                                 .h=sprite_PI->size }
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
              SDL_Texture *tex_PI,
              SDL_Texture *tex_PW)
{
    IMG_Quit();
    SDL_DestroyTexture(bgnd_tex);
    SDL_DestroyTexture(tex_PI);
    SDL_DestroyTexture(tex_PW);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_CloseFont(debug_font);
    TTF_Quit();
    SDL_Quit();
}

typedef struct
{
    int x;
    int y;
} Character;

void center_char_on_screen(Character *character, int size, WindowInfo wI)
    { // Center sprite on the screen
        character->x=(wI.w-size)/2;
        character->y=(wI.h-size)/2;
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
            shutdown(NULL, NULL, NULL, NULL, NULL, NULL);
            return EXIT_FAILURE;
        }
        if(  font_load(&debug_font, "fonts/ProggyClean.ttf", 16) < 0  ) // Load font
        {
            shutdown(NULL, NULL, NULL, NULL, NULL, NULL);
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
        shutdown(debug_font, ren, win, NULL, NULL, NULL); return EXIT_FAILURE;
    }

    // Load the spritesheets
    IMG_Init(IMG_INIT_PNG);                                     // Spritesheet is a PNG

    Sprite PenguinIdle = {.path = "art/penguin-huff.png"};
    Sprite PenguinWalk = {.path = "art/penguin-waddle.png"};

    Sprite *sprite_PI = &PenguinIdle;                           // _PI : Penguin Idle
    if(  sprite_load_info(sprite_PI) < 0  )
    {
        shutdown(debug_font, ren, win, NULL, NULL, NULL);
        return EXIT_FAILURE;
    }

    Sprite *sprite_PW = &PenguinWalk;                           // _PW : Penguin Walk
    if(  sprite_load_info(sprite_PW) < 0  )
    {
        shutdown(debug_font, ren, win, NULL, NULL, NULL);
        return EXIT_FAILURE;
    }
    sprite_PW->ticks_per_frame = 8;

    Character char_penguin = {.x=0, .y=0};
    center_char_on_screen(&char_penguin, sprite_PI->scale*sprite_PI->size, wI);

    SDL_Texture *tex_PI;                                       // One texture for entire sprite sheet
    { // Load Texture directly from spritesheet file
        tex_PI = IMG_LoadTexture(ren, sprite_PI->path);
        if(  tex_PI == NULL  )
        {
            printf("Failed to load \"%s\": %s", sprite_PI->path, IMG_GetError());
            SDL_DestroyWindow(win); SDL_DestroyRenderer(ren);
            TTF_CloseFont(debug_font); TTF_Quit(); SDL_Quit();
            return EXIT_FAILURE;
        }
    }
    SDL_Texture *tex_PW;                                       // One texture for entire sprite sheet
    { // Load Texture directly from spritesheet file
        tex_PW = IMG_LoadTexture(ren, sprite_PW->path);
        if(  tex_PW == NULL  )
        {
            printf("Failed to load \"%s\": %s", sprite_PW->path, IMG_GetError());
            SDL_DestroyWindow(win); SDL_DestroyRenderer(ren);
            TTF_CloseFont(debug_font); TTF_Quit(); SDL_Quit();
            return EXIT_FAILURE;
        }
    }

    // Create a background texture with a sky-colored gradient
    SDL_Texture *bgnd_tex;
    bgnd_gradient(&bgnd_tex, ren, wI);


    // Game state
    bool quit = false;
    bool show_debug = true;
    bool walk_animation = false;
    int walk_direction = 0;
    int ticks = 0;                                              // Count SDL ticks
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
        // Penguin position
        sprite_PW->render.x = char_penguin.x;
        sprite_PI->render.x = char_penguin.x;
        sprite_PW->render.y = char_penguin.y;
        sprite_PI->render.y = char_penguin.y;
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
                    sprite_PI->scale++;
                    if(  sprite_PI->scale>32  ) sprite_PI->scale=32;
                    sprite_PI->render.x=(wI.w-sprite_PI->scale*sprite_PI->size)/2;
                    sprite_PI->render.y=(wI.h-sprite_PI->scale*sprite_PI->size)/2;
                    sprite_PI->render.w=sprite_PI->scale*sprite_PI->size;
                    sprite_PI->render.h=sprite_PI->scale*sprite_PI->size;
                }
                if(  k[SDL_SCANCODE_DOWN]  )
                {
                    sprite_PI->scale--;
                    if(  sprite_PI->scale<1  ) sprite_PI->scale=1;
                    sprite_PI->render.x=(wI.w-sprite_PI->scale*sprite_PI->size)/2;
                    sprite_PI->render.y=(wI.h-sprite_PI->scale*sprite_PI->size)/2;
                    sprite_PI->render.w=sprite_PI->scale*sprite_PI->size;
                    sprite_PI->render.h=sprite_PI->scale*sprite_PI->size;
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
                        case SDLK_RIGHT:
                            walk_animation = true;
                            walk_direction = 1;
                            if(  kmod & (KMOD_LSHIFT|KMOD_RSHIFT)  )
                            { // DEBUG
                                anim_next_frame(&sprite_PI->framenum, sprite_PI->framecnt);
                                anim_load_frame(&sprite_PI->frame, sprite_PI->size, sprite_PI->framenum);
                            }
                            break;
                        case SDLK_LEFT:
                            walk_animation = true;
                            walk_direction = -1;
                            if(  kmod & (KMOD_LSHIFT|KMOD_RSHIFT)  )
                            { // DEBUG
                                anim_prev_frame(&sprite_PI->framenum, sprite_PI->framecnt);
                                anim_load_frame(&sprite_PI->frame, sprite_PI->size, sprite_PI->framenum);
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
                                walk_animation = false;
                            }
                            break;
                    }
                }
            }
        }

        { // Animate
            Sprite *sprite = walk_animation ? sprite_PW : sprite_PI;
            if(  ticks < sprite->ticks_per_frame  ) ticks++;
            else
            {
                anim_next_frame(&sprite->framenum, sprite->framecnt);
                anim_load_frame(&sprite->frame, sprite->size, sprite->framenum);
                ticks = 0;
            }
            if(  walk_animation  ) char_penguin.x += 1*sprite->scale*walk_direction;
        }

        // Render
        { // Paint over old video frame with a beautiful background gradient
            SDL_RenderCopy(ren, bgnd_tex, NULL, NULL);
        }
        { // Draw the sprite
            /* SDL_RenderCopy(ren, tex_PI, NULL, NULL);           // Draw entire spritesheet */
            Sprite *sprite = (walk_animation) ? sprite_PW : sprite_PI;
            SDL_Texture *tex = (walk_animation) ? tex_PW : tex_PI;
            SDL_RendererFlip flip = (walk_direction==1) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
            /* SDL_RenderCopy(ren, tex, &sprite->frame, &sprite->render); // Draw one frame */
            SDL_RenderCopyEx(ren, tex, &sprite->frame, &sprite->render, 0, NULL, flip); // Draw one frame
        }
        if(show_debug)
        { // Debug overlay
            Sprite *sprite = (walk_animation) ? sprite_PW : sprite_PI;
            { // Put text in the text box
                char *d = tb.text;                              // d : see macro "print"
                print("Spritesheet: "); print(sprite->path);
                print(" | ");
                print("Sprite size: "); printint(4, sprite->size); print("x"); printint(4, sprite->size);
                print(" | ");
                print("Animation frame: "); printint(3, sprite->framenum); print(" / "); printint(3, sprite->framecnt);
                print(" | ");
                print("Ticks per frame: "); printint(3, sprite->ticks_per_frame);
                print(" | ");
                print("Animation: "); if(walk_animation){ print("waddle");} else print("huff");
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

    shutdown(debug_font, ren, win, bgnd_tex, tex_PI, tex_PW);
    return EXIT_SUCCESS;
}

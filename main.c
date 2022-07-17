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

#define print(str) { const char *c = str; while(*c!='\0'){*d++=*c++;} *d='\0'; } // c walks src, d walks dst
#define printint(ndig,val) { char str[ndig+1]; sprintf(str, "%d", val); print(str); }

void load_frame(SDL_Rect *frame, const int size, const int framenum)
{ // Load frame rect with rectangle that bounds the current animation frame
    int col = (framenum-1) % 8;                             // col 0 is frame 1, col 7 is frame 8
    int row = (int)((framenum-1)/8);                        // row 0 has frames 1 to 8
    frame->x = col*size; frame->y = row*size;
}
void prev_frame(int *framenum, const int framecnt)
{ // Load previous frame number
    /* *************DOC***************
     * framenum : current frame number
     * framecnt : total number of frames in this spritesheet
     * *******************************/
    *framenum -= 1;
    if(  *framenum < 1  ) *framenum = framecnt;             // Frame 1 - 1 = Frame framecnt
}
void next_frame(int *framenum, const int framecnt)
{ // Load next frame number
    /* *************DOC***************
     * framenum : current frame number
     * framecnt : total number of frames in this spritesheet
     * *******************************/
    *framenum += 1;
    if(  *framenum > framecnt  ) *framenum = 1;             // Frame framecnt + 1 = Frame 1
}
void OLD_next_frame(SDL_Rect *frame, bool *run, int sprite_size, int nframes)
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
    if(  SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND) <0  )     // Draw with alpha
    {
        puts("Cannot draw with alpha channel");
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Texture *bgnd_tex;
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
        bgnd_tex = SDL_CreateTextureFromSurface(ren, surf);
        SDL_SetTextureBlendMode(bgnd_tex, SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(surf);
    }

    // Turn spritesheet into sprite animation
    IMG_Init(IMG_INIT_PNG);
    SDL_Texture *img_tex;                                       // One texture for entire sprite sheet
    int img_framenum = 1;
    /* const char *img_path = "art/Kerby blinkey tiredey.png"; const int img_framecnt = 30; */
    /* const int sprite_size=64;                                   // Sprite frames are 64x64 */
    const char *img_path = "art/penguin-huff.png"; const int img_framecnt = 12;
    const int sprite_size=64;                                   // Sprite frames are 64x64
    /* const char *img_path = "art/Snowsis Running.png"; const int img_framecnt = 13; */
    /* const int sprite_size=32;                                   // Sprite frames are 32x32 */
    /* const char *img_path = "art/numbers.png"; const int img_framecnt = 9; */
    /* const int sprite_size=32;                                   // Sprite frames are 32x32 */
    int sprite_scale = 1;                                       // Initial scale is 1x actual size
    SDL_Rect sprite_render = {.x=(wI.w-sprite_scale*sprite_size)/2, // Initial x-pos: center
                              .y=(wI.h-sprite_scale*sprite_size)/2, // Initial y-pos: center
                              .w=sprite_scale*sprite_size,      // Scale sprite up by 4x
                              .h=sprite_scale*sprite_size       // Scale sprite up by 4x
                              };
    SDL_Rect sprite_frame = {  .x=0, .y=0,                             // start at first frame
                        .w=sprite_size, .h=sprite_size          // 64x64 sprite
                        };
    { // Load Texture directly from spritesheet file
        img_tex = IMG_LoadTexture(ren, img_path);
        if(  img_tex == NULL  )
        {
            printf("Failed to load \"%s\": %s", img_path, IMG_GetError());
            SDL_DestroyWindow(win); SDL_DestroyRenderer(ren);
            TTF_CloseFont(font); TTF_Quit(); SDL_Quit();
            return EXIT_FAILURE;
        }
    }
    { // Load spritesheet into a Surface to check background is transparent
        SDL_Surface *img_surf = IMG_Load(img_path);
        if(  img_surf == NULL  )
        { // Unable to load image
            printf("Failed to load \"%s\": %s", img_path, IMG_GetError());
            SDL_DestroyWindow(win); SDL_DestroyRenderer(ren);
            TTF_CloseFont(font); TTF_Quit(); SDL_Quit();
            return EXIT_FAILURE;
        }
        uint32_t *p = img_surf->pixels;
        if(  *p != 0x00000000  )
        { // Top-left pixel in image is not 0 -- background is not transparent
            printf("Sprite sheet \"%s\" does not have a transparent background.", img_path);
            SDL_FreeSurface(img_surf);
            SDL_DestroyWindow(win); SDL_DestroyRenderer(ren);
            TTF_CloseFont(font); TTF_Quit(); SDL_Quit();
            return EXIT_FAILURE;
        }
        SDL_FreeSurface(img_surf);
    }

    // Game state
    bool quit = false;
    bool show_debug = false;
    int run_animation = 0;
    bool idle_animation = false;
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
                            run_animation = 2*img_framecnt;     // Run animation twice
                            ticks_per_anim_frame = 4;           // Reset animation speed
                            break;
                        case SDLK_RIGHT:
                            idle_animation = false;
                            if(0)
                            { // DEBUG
                                next_frame(&img_framenum, img_framecnt);
                                load_frame(&sprite_frame, sprite_size, img_framenum);
                            }
                            break;
                        case SDLK_LEFT:
                            idle_animation = false;
                            if(0)
                            { // DEBUG
                                prev_frame(&img_framenum, img_framecnt);
                                load_frame(&sprite_frame, sprite_size, img_framenum);
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
                            idle_animation = true;
                            start_penguin_huff = true;
                            delay_btwn_huffs = img_framecnt;
                            ticks_per_anim_frame = 2;           // Initial animation speed
                            img_framenum = 0;                   // Reset idle animation
                            break;
                    }
                }
            }
        }
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

        if(  idle_animation  )
        {
            if(  start_penguin_huff  )
            {
                run_animation = 2*img_framecnt;     // Run animation twice
                if (ticks_per_anim_frame > 4) ticks_per_anim_frame = 4; // Clamp at 8
                start_penguin_huff = false;
            }
            if(  run_animation > 0  )
            {
                if(  ticks < ticks_per_anim_frame  ) ticks++;
                else
                {
                    /* OLD_next_frame(&sprite_frame, &run_animation, sprite_size, img_framecnt); // 30 frames */
                    next_frame(&img_framenum, img_framecnt);
                    load_frame(&sprite_frame, sprite_size, img_framenum);
                    ticks = 0;
                    run_animation--;                                // Count down num times to run animation
                    if(  (run_animation%img_framecnt) == 0  )
                    {
                        ticks_per_anim_frame++;                     // Slow down animation to give is some variation
                    }
                }
            }
            else
            {
                ticks++;
                if( ticks > delay_btwn_huffs )
                {
                    ticks = 0;
                    start_penguin_huff = true;
                    delay_btwn_huffs += img_framecnt;
                }
            }
        }

        // Render
        { // Paint over old video frame with a beautiful background gradient
            SDL_RenderCopy(ren, bgnd_tex, NULL, NULL);
        }
        if(0)
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
                char *d = tb.text;                              // d : see macro "print"
                print("Spritesheet: "); print(img_path);
                print(" | ");
                print("Animation frame: "); printint(3, img_framenum); print(" / "); printint(3, img_framecnt);
                print(" | ");
                print("Window size: "); printint(5, wI.w); print("x"); printint(5, wI.h); print(" (wxh)");
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
    SDL_DestroyTexture(bgnd_tex);
    SDL_DestroyTexture(img_tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    return EXIT_SUCCESS;
}

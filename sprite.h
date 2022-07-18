#ifndef __SPRITE_H__
#define __SPRITE_H__

#include <SDL.h>
#include <SDL_image.h>

typedef struct
{
    const char *path;                   // Path to sprite sheet
    int size;                           // Detect from sprite sheet : Ex: 64x64
    int framecnt;                       // Detect from sprite sheet : Ex: 8 frames
    int framenum;                       // Current frame number : 1 to framecnt
    int scale;                          // Scale sprite by this amount
    SDL_Rect render;                    // Determines size and location of rendered sprite
    SDL_Rect frame;                     // Selects frame (from sprite sheet) to render
} Sprite;

bool sprite_sheet_has_transparency(SDL_Surface *sprite_surf, const char *sprite_path)
{ // Return true if sprite_sheet has transparency, otherwise return false
    uint32_t *p = sprite_surf->pixels;
    if(  *p != 0x00000000  )
    { // Top-left pixel in image is not 0 -- background is not transparent
        printf("Sprite sheet \"%s\" does not have a transparent background.", sprite_path);
        return false;
    }
    return true;
}

int sprite_get_size(SDL_Surface *sprite_surf)
{ // Return size : e.g., 64 for a 64x64
    /* *************DOC***************
     * - Sprites are always square, so a single number is enough to represent size
     * - Sprite sheet must have eight sprite frames per row
     * *******************************/
    return sprite_surf->w/8;                                    // e.g., 512/8 = 64
}

int sprite_get_num_frames(SDL_Surface *sprite_surf, int sprite_size)
{ // Return number of frames in the spritesheet
    /* *************DOC***************
     * Determine number of frames by counting the number of non-empty frames.
     * The count ends with the first empty frame.
     * An empty frame has the value 0 for all pixels.
     * *******************************/
    int sprite_framecnt = 0;
    printf("sprite_surf->\n\t%dx%d (wxh)\n\tpitch: %d\n", sprite_surf->w, sprite_surf->h, sprite_surf->pitch);
    printf("sprite_surf->format->\n\tBytesPerPixel: %d\n", sprite_surf->format->BytesPerPixel);
    fflush(stdout);
    // Each pixel is 4 bytes
    // Each row is 512 pixels (2048 bytes)
    bool found_empty_frame = false;
    int cstart = 0; int rstart = 0;
    int cstop = sprite_size; int rstop = sprite_size;
    uint32_t sum = 0;                                           // Sum pixel values
    uint32_t *p = sprite_surf->pixels;                          // Walk the pixels
    uint32_t *prow;                                             // Point at first pixel in the row
    uint32_t *p0 = sprite_surf->pixels;                         // Point at first pixel in the image
    while( found_empty_frame == false)
    {
        for( int r=rstart; r<rstop; r++)
        {
            prow = p0 + r*sprite_surf->w;                       // prow : point at col 0 in this row
            for( int c=cstart; c<cstop; c++)
            {
                sum += *p;                                      // Accumulate
                p = prow + c;                                   // Advance to next column
            }
        }
        if(  sum == 0  ) found_empty_frame = true;              // Found an empty frame. Count is done.
        else
        {
            sprite_framecnt += 1;                               // Frame not empty. Count it.
            sum = 0;                                            // Reset the sum.
            cstart += sprite_size;                              // Advance to next frame.
            if(  cstart >= sprite_surf->w  )                    // If this is the end of the row
            {
                cstart = 0;                                     // Go back to column 0
                rstart += sprite_size;                          // and go to next row
                rstop = rstart + sprite_size;
            }
            cstop = cstart + sprite_size;
        }
        // If number of frames is a multiple of 8, there are no empty frames.
        // Instead, look for when the row goes past the bottom of the spritesheet.
        if(  rstart >= sprite_surf->h  ) found_empty_frame = true;
    }
    return sprite_framecnt;
}

int sprite_load_info(Sprite *sprite)
{ // Auto-detect sprite size and number of frames of animation
    // Load spritesheet into a Surface to check transparency, size, and number of frames
    SDL_Surface *sprite_surf = IMG_Load(sprite->path);
    if(  sprite_surf == NULL  )
    { // Unable to load image
        printf("Failed to load \"%s\": %s", sprite->path, IMG_GetError());
        return -1;
    }
    if(  sprite_sheet_has_transparency(sprite_surf, sprite->path) == false )
    { // Sprite sheet does not have a transparent background
        SDL_FreeSurface(sprite_surf);
        return -1;
    }
    sprite->size = sprite_get_size(sprite_surf);            // Determine size of sprite
    sprite->framecnt = 0;                                   // Determine number of frames in animation
    sprite->framecnt = sprite_get_num_frames(sprite_surf, sprite->size);
    SDL_FreeSurface(sprite_surf);

    // Load other values
    sprite->framenum = 1;                                   // Start animation at first frame
    sprite->scale = 2;                                      // Initial scale is 2x actual size
    sprite->render = (SDL_Rect){.x=0,.y=0,
                                .w=sprite->scale*sprite->size, // Scale sprite up by 2x
                                .h=sprite->scale*sprite->size  // Scale sprite up by 2x
                                };
    sprite->frame  = (SDL_Rect){.x=0, .y=0,                 // start at first frame
                                .w=sprite->size, .h=sprite->size // 64x64 sprite
                                };
    return 0;
}

#endif // __SPRITE_H__


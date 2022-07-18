#ifndef __SPRITE_H__
#define __SPRITE_H__

#include <SDL.h>
#include <SDL_image.h>

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

#endif // __SPRITE_H__


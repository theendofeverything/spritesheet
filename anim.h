#ifndef __ANIM_H__
#define __ANIM_H__
#include <SDL.h>

void anim_load_frame(SDL_Rect *frame, const int size, const int framenum)
{ // Load frame rect with rectangle that bounds the current animation frame
    int col = (framenum-1) % 8;                             // col 0 is frame 1, col 7 is frame 8
    int row = (int)((framenum-1)/8);                        // row 0 has frames 1 to 8
    frame->x = col*size; frame->y = row*size;
}
void anim_prev_frame(int *framenum, const int framecnt)
{ // Load previous frame number
    /* *************DOC***************
     * framenum : current frame number
     * framecnt : total number of frames in this spritesheet
     * *******************************/
    *framenum -= 1;
    if(  *framenum < 1  ) *framenum = framecnt;             // Frame 1 - 1 = Frame framecnt
}
void anim_next_frame(int *framenum, const int framecnt)
{ // Load next frame number
    /* *************DOC***************
     * framenum : current frame number
     * framecnt : total number of frames in this spritesheet
     * *******************************/
    *framenum += 1;
    if(  *framenum > framecnt  ) *framenum = 1;             // Frame framecnt + 1 = Frame 1
}

#endif // __ANIM_H__


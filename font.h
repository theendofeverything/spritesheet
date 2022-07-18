#ifndef __FONT_H__
#define __FONT_H__

int font_init(void)
{
    if(  TTF_Init() < 0  )
    { // Error handling: Cannot initialize SDL_ttf
        puts("Cannot initialize SDL_ttf");
        return -1;
    }
    return 0;
}
int font_load(TTF_Font **font_struct, const char *font_path, int ptsize)
{
    *font_struct = TTF_OpenFont(font_path, ptsize);
    if(  *font_struct == NULL  )
    { // Error handling: font file does not exist
        printf("Cannot open font file. Please check \"%s\" exists.", font_path);
        return -1;
    }
    return 0;
}

#endif // __FONT_H__


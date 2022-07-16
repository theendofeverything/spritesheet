/* *************DOC***************
 * Example
 * -------
 * ./test-transparency.exe art/Kerby\ blinkey\ tiredey.png
 *
 * Pixel in top-left corner is:
 * - 0x00000000 if transparency is preserved
 * - 0xFFFFFFFF if transparency is lost
 * *******************************/
#include <SDL.h>
#include <SDL_image.h>

int main(int argc, char *argv[])
{
    for(int i=0; i<argc; i++) puts(argv[i]);

    const char *img_path;
    if(argc>1) img_path = argv[1];
    else img_path = "art/Snowsis Running.png";

    // Setup
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *win = SDL_CreateWindow("", 0,0,100,100,0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, 0);

    IMG_Init(IMG_INIT_PNG);
    { // Create surface to inspect image pixel values
        SDL_Surface *img_surf = IMG_Load(img_path);
        if(  img_surf == NULL  )
        {
            printf("Failed to load \"%s\": %s", img_path, IMG_GetError());
            SDL_DestroyWindow(win); SDL_DestroyRenderer(ren); SDL_Quit();
            return EXIT_FAILURE;
        }
        // Tell me about the first pixel in the image:
        uint32_t *p = img_surf->pixels;
        printf("Pixel in top-left corner of image: 0x%08X\n",*p); // Yes, I get 0xFFFFFFFF, the problem is Pixaki
        printf("Image transparency is %s.\n", (*p==0)?"PRESERVED":"LOST");
        SDL_FreeSurface(img_surf);
    }

    // Shutdown
    IMG_Quit();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return EXIT_SUCCESS;
}

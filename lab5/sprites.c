#include <lcom/lcf.h>
#include "sprites.h"
#include "video.h"

void draw_xpm(const char *xpm[], uint16_t xi, uint16_t yi)
{
    int width, height;
    char *map = read_xpm(xpm, &width, &height);
    for (size_t y = yi; y < (unsigned int) yi + height; y++)
    {
        for (size_t x = xi; x < (unsigned int) xi + width; x++)
        {
          vg_draw_pixel(x, y, map[(y-yi)*width+(x-xi)]);
        }
    }
}

Sprite* create_sprite (const char *pic[], int x, int y, int xs, int ys)
{
    //allocate space for the "object"
    Sprite *spr = (Sprite *) malloc ( sizeof(Sprite));
    if(spr == NULL )
        return NULL;
    // read the sprite pixmap
    spr->map = read_xpm(pic, &(spr->width), &(spr->height));
    if( spr->map == NULL ) 
    {
        free(spr);
        return NULL;
    }
    spr->x = x;
    spr->y = y;
    spr->xspeed = xs;
    spr->yspeed = ys;
    return spr;
}

void draw_sprite (Sprite * spr)
{
    for (size_t y = spr->y; y < (unsigned int) spr->y + spr->height; y++)
    {
        for (size_t x = spr->x; x < (unsigned int) spr->x + spr->width; x++)
        {
          vg_draw_pixel(x, y, spr->map[(y-spr->y)*spr->width+(x-spr->x)]);
        }
    }
}

void blacken_sprite(Sprite * spr)
{
    vg_draw_rectangle(spr->x, spr->y, spr->width, spr->height, 0);
}

void delete_sprite(Sprite* spr)
{
    vg_draw_rectangle(spr->x, spr->y, spr->width, spr->height, 0);
    free(spr->map);
    free(spr);
}

#include <lcom/lcf.h>
#include <stdarg.h>
#include "sprites.h"
#include "bmp.h"
#include "video.h"

Sprite* create_sprite (Bitmap * bmp, int x, int y, int xspeed, int yspeed)
{
    //allocate space for the "object"
    Sprite *spr = (Sprite *) malloc ( sizeof(Sprite));
    if(spr == NULL )
        return NULL;
    spr->bmp = bmp;
    spr->x = x;
    spr->y = y;
    spr->width = spr->bmp->bitmapInfoHeader.width;
    spr->height = spr->bmp->bitmapInfoHeader.height;
    spr->xspeed = xspeed;
    spr->yspeed = yspeed;
    return spr;
}

Animated_Sprite * create_animated_sprite(int x, int y, int xspeed, int yspeed, unsigned numAnim, ...)
{
    //allocate space for the "object"
    Animated_Sprite *spr = (Animated_Sprite *) malloc (sizeof(Animated_Sprite));
    if(spr == NULL )
        return NULL;
    spr->x = x;
    spr->y = y;
    spr->xspeed = xspeed;
    spr->yspeed = yspeed;
    spr->current_animation = 0;
    spr->animations = numAnim;
    va_list valist;
    va_start(valist, numAnim);
    spr->bmps = (Bitmap **) malloc(numAnim*sizeof(Bitmap *));
    for (unsigned i = 0; i < numAnim; i++) {
      spr->bmps[i] = va_arg(valist, Bitmap *);
    }
    spr->width = spr->bmps[0]->bitmapInfoHeader.width;
    spr->height = spr->bmps[0]->bitmapInfoHeader.height;
    va_end(valist);
    return spr;
}

int draw_animated_sprite (Animated_Sprite * spr, bool check_col)
{
    if(spr->y < 100)
        spr->y = 100;
    uint32_t pixelColorReturn;
    spr->current_animation++;
    spr->current_animation = spr->current_animation % spr->animations;
    if((pixelColorReturn = drawBitmap(spr->bmps[spr->current_animation], spr->x, spr->y, check_col)))
        return pixelColorReturn;
    else
        return 0;
}

int draw_sprite (Sprite * spr, bool check_col)
{
    if(spr->y < 100)
        spr->y = 100;
    uint32_t pixelColorReturn;
    if((pixelColorReturn = drawBitmap(spr->bmp, spr->x, spr->y, check_col)))
        return pixelColorReturn;
    else
        return 0;
}

int move_sprite (Sprite * spr, bool check_col)
{
    if(spr->y < 100)
        return 1;    
    spr->y+=spr->yspeed;
    spr->x+=spr->xspeed;
    if(check_col)
    {
        uint32_t pixelColor;
        if((pixelColor = drawBitmap(spr->bmp, spr->x, spr->y, true)))
        {
            drawBitmap(spr->bmp, spr->x, spr->y, false);
            return pixelColor;
        }
    }
    else
    {
        return drawBitmap(spr->bmp, spr->x, spr->y, false);
    }
    return 0;
}

int move_animated_sprite (Animated_Sprite * spr, bool check_col)
{
    if(spr->y < 100)
        return 1;    
    spr->y+=spr->yspeed;
    spr->x+=spr->xspeed;
    spr->current_animation++;
    spr->current_animation = spr->current_animation % spr->animations;
    if(check_col)
    {
        uint32_t pixelColor;
        if((pixelColor = drawBitmap(spr->bmps[spr->current_animation], spr->x, spr->y, true)))
        {
            drawBitmap(spr->bmps[spr->current_animation], spr->x, spr->y, false);
            return pixelColor;
        }
    }
    else
    {
        return drawBitmap(spr->bmps[spr->current_animation], spr->x, spr->y, false);
    }
    return 0;
}

void hide_sprite(Sprite * spr)
{
    hideBitmap(spr->bmp, spr->x, spr->y);
}

void hide_animated_sprite(Animated_Sprite * spr)
{
    hideBitmap(spr->bmps[spr->current_animation], spr->x, spr->y);
}

void delete_sprite(Sprite* spr)
{
    free(spr);
}

void delete_animated_sprite (Animated_Sprite * spr)
{
    free(spr);
}

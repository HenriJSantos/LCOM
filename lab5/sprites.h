#pragma once

typedef struct {
  int x,y;             /**< current sprite position */
  int width, height;   /**< sprite dimensions */
  int xspeed, yspeed;  /**< current speeds in the x and y direction */
  char *map;           /**< the sprite pixmap (use read_xpm()) */
} Sprite;

void draw_xpm (const char *xpm[], uint16_t xi, uint16_t yi);

Sprite* create_sprite (const char *pic[], int x, int y, int xs, int ys);

void draw_sprite (Sprite * spr);

void blacken_sprite(Sprite * spr);

void delete_sprite (Sprite* spr);

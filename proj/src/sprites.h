#ifndef SPRITES_H
#define SPRITES_H

#include <lcom/lcf.h>
#include "bmp.h"

/** @defgroup Sprites Sprites
 * @{
 *
 * @brief Module for sprite-oriented image manipulation
 */

/**
 * @brief General Sprite struct
 */
typedef struct {
  int x,y;             /*!< current sprite position */
  int width, height;   /*!< sprite dimensions */
  int xspeed, yspeed;  /*!< current speeds in the x and y direction */
  Bitmap * bmp;        /*!< the sprite pixmap (use read_xpm()) */
} Sprite;

/**
 * @brief General Animated Sprite struct
 */
typedef struct {
  int x,y;                    /*!< current sprite position */
  int width, height;          /*!< sprite dimensions */
  int xspeed, yspeed;         /*!< current speeds in the x and y direction */
  unsigned animations;        /*!< Number of animation frames */
  unsigned current_animation; /*!< Which animation the sprite is on */
  Bitmap ** bmps;             /*!< Array of bitmaps */
} Animated_Sprite;

/**
 * @brief Creates a sprite with specified characteristics
 * 
 * @param bmp Sprite's Bitmap
 * @param x Horizontal position
 * @param y Vertical position
 * @param xspeed Horizontal speed
 * @param yspeed Horizontal speed
 * 
 * @return pointer to created sprite, or NULL if any errors occur
 */
Sprite* create_sprite (Bitmap * bmp, int x, int y, int xspeed, int yspeed);

/**
 * @brief Creates an animated sprite with specified characteristics
 * 
 * @param x Horizontal position
 * @param y Vertical position
 * @param xspeed Horizontal speed
 * @param yspeed Horizontal speed
 * @param numAnim number of animations supplied to the animated sprite
 * 
 * @return pointer to created animated sprite, or NULL if any errors occur
 */
Animated_Sprite * create_animated_sprite(int x, int y, int xspeed, int yspeed, unsigned numAnim, ...);

/**
 * @brief Draws a sprite
 * 
 * @param spr Sprite to move
 * @param check_col True if checking collision, false otherwise
 * 
 * @return 0 if no errors occur, color of sprite if there's a collision, 1 otherwise
 */
int draw_sprite (Sprite * spr, bool check_col);

/**
 * @brief Draws an animated sprite
 * 
 * @param spr Animated Sprite to move
 * @param check_col True if checking collision, false otherwise
 * 
 * @return 0 if no errors occur, color of sprite if there's a collision, 1 otherwise
 */
int draw_animated_sprite (Animated_Sprite * spr, bool check_col);

/**
 * @brief Moves a sprite sprite
 * 
 * @param spr Animated Sprite to move
 * @param check_col True if checking collision, false otherwise
 * 
 * @return 0 if no errors occur, color of sprite if there's a collision, 1 otherwise
 */
int move_sprite (Sprite * spr, bool check_col);

/**
 * @brief Moves an animated sprite sprite
 * 
 * @param spr Animated Sprite to move
 * @param check_col True if checking collision, false otherwise
 * 
 * @return 0 if no errors occur, color of sprite if there's a collision, 1 otherwise
 */
int move_animated_sprite (Animated_Sprite * spr, bool check_col);

/**
 * @brief Hides a sprite
 * 
 * @param spr Sprite to hide
 */
void hide_sprite(Sprite * spr);

/**
 * @brief Hides an animated sprite
 * 
 * @param spr Animated Sprite to hide
 */
void hide_animated_sprite(Animated_Sprite * spr);

/**
 * @brief Deletes a sprite from memory
 * 
 * @param spr Sprite to delete
 */
void delete_sprite (Sprite* spr);

/**
 * @brief Deletes an animated sprite from memory
 * 
 * @param spr Animated Sprite to delete
 */
void delete_animated_sprite (Animated_Sprite * spr);

#endif

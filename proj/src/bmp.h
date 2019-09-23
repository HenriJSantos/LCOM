#ifndef BITMAP_H
#define BITMAP_H

#include <lcom/lcf.h>

/** @defgroup Bitmaps Bitmaps
 * @{
 *
 * @brief Module for bitmap display and manipulation
 */

/*
 * @brief Color used for the crosshair
 */
#define CROSSHAIR_COLOR 0xffc10100

/*
 * @brief Bitmap file header
 */
typedef struct {
	uint16_t type;		/*!< Specifies the file type */
	uint32_t size;		/*!< Specifies the size in bytes of the bitmap file */
	uint32_t reserved;	/*!< Reserved; must be 0 */
	uint32_t offset;	/*!< Specifies the offset in bytes from the bitmapfileheader to the bitmap bits */
} BitmapFileHeader;

/*
 * @brief Bitmap info header
 */
typedef struct {
	uint32_t size;				/*!< Specifies the number of bytes required by the struct */
	uint32_t width;				/*!< Specifies width in pixels */
	uint32_t height;			/*!< Specifies height in pixels */
	uint16_t planes;			/*!< Specifies the number of color planes, must be 1 */
	uint16_t bits_per_pixel;	/*!< Specifies the number of bit per pixel */
	uint32_t compression;		/*!< Specifies the type of compression */
	uint32_t image_size;		/*!< Size of image in bytes */
	uint32_t x_res;				/*!< Number of pixels per meter in x axis */
	uint32_t y_res;				/*!< Number of pixels per meter in y axis */
	uint32_t num_colors;		/*!< Number of colors used by the bitmap */
	uint32_t imp_colors;		/*!< Number of colors that are important */
} BitmapInfoHeader;

/*
 * @brief Represents a bitmap
 */
typedef struct {
	BitmapInfoHeader bitmapInfoHeader;
	uint32_t * bitmapData;
} Bitmap;

/**
 * @brief Sets a global variable "background" to the bitmap provided, for collision-detection purposes
 *
 * @param bg pointer to bitmap to set as background
 */
void setBackground(Bitmap * bg);

/**
 * @brief Loads a bitmap from a file to memory.
 *
 * @param filename Name/directory of bitmap to load.
 * 
 * @return Pointer to bitmap if successful, and NULL otherwise.
 */
Bitmap* loadBitmap(const char* filename);

/**
 * @brief Draws a bitmap in specified location.
 *
 * @param bitmap Bitmap to draw
 * @param x Horizontal position to start drawing
 * @param y Vertical position to start drawing
 * @param check_col Bool that specifies if collisions should be taken into account
 * 
 * @return 0 if successful, 1 if non-collision error occurs, and the color of the pixel that caused a collision if such occurs
 */
int drawBitmap(Bitmap* bitmap, int x, int y, bool check_col);

/**
 * @brief Hides a bitmap in specified location, drawing the background over it
 *
 * @param bmp Bitmap to hide
 * @param x Horizontal position where the bitmap starts
 * @param y Vertical position where the bitmap starts
 */
void hideBitmap(Bitmap * bmp, int x, int y);

/**
 * @brief Frees a bitmap from memory
 *
 * @param bmp Bitmap to free
 */
void deleteBitmap(Bitmap* bmp);

#endif

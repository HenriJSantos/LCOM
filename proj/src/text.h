#pragma once

/** @defgroup Serial_Port Serial_Port
 * @{
 *
 * @brief Module for text-related game operations
 */

/**
 * @brief Imports number bitmaps to memory
 */
void init_numbers();

/**
 * @brief Prints int on screen
 * 
 * @param num Int to print
 * @param x Horizontal position
 * @param y Vertical position
 */
void print_int(unsigned num, unsigned x, unsigned y);

/**
 * @brief Hides int on screen
 * 
 * @param num Int to hide
 * @param x Horizontal position
 * @param y Vertical position
 */
void hide_int(unsigned num, unsigned x, unsigned y);

/**
 * @brief Deletes numbers from memory
 */
void free_numbers();

/**
 * @brief Prints score to file with date and hour
 * 
 * @param score Score obtained to print
 */
void print_score(int score);

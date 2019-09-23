#pragma once

#include <lcom/lcf.h>
#include "keyboard.h"
#include "i8254.h"
#include "i8042.h"

/** @defgroup Keyboard Keyboard
 * @{
 *
 * @brief Module for interaction with keyboard
 */

/**
 * @brief Subscribes to keyboard interrupts
 */
int (keyboard_subscribe_int)(uint8_t *bit_no);

/**
 * @brief Unsubscribes to keyboard interrupts
 */
int (keyboard_unsubscribe_int)();

/**
 * @brief Checks if there is a full scancode ready to be printed
 *
 * @param scancodeArray pointer to the current array of scancodes
 * @param size pointer to current array size
 * 
 * @return true if there's a full scancode ready to be printed, false otherwise
 */
bool (gotFullScancode)(uint8_t scancodeArray[], uint8_t *size);

/**
 * @brief Polling function that checks for OBF and calls kbc_ih if there's something in the buffer
 */
int (readScancode)();

/**
 * @brief Keyboard interrupt handler
 */
void (kbc_ih)();

/**
 * @brief Reenables default keyboard interrupts once polling is done
 */
int (enableKeyboardInterrupts)();

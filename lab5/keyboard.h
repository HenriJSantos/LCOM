#pragma once

#include <lcom/lcf.h>
#include "keyboard.h"
#include "i8254.h"
#include "i8042.h"

int (keyboard_subscribe_int)(uint8_t *bit_no); // Subscribes to keyboard interrupts

int (keyboard_unsubscribe_int)(); // Unsubscribes to keyboard interrupts

bool (gotFullScancode)(uint8_t scancodeArray[], uint8_t *size); // Returns true if there is a full scancode ready to be printed

int (readScancode)(); // Polling function that checks for OBF and calls kbc_ih if there's something in the buffer

void (kbc_ih)(); // Keyboard interrupt handler

int (enableKeyboardInterrupts)(); // Reenables default keyboard interrupts once polling is done

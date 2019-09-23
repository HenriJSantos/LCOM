#pragma once
#include <lcom/lcf.h>
#include "UTIL.h"

/** @defgroup Serial_Port Serial_Port
 * @{
 *
 * @brief Module for interaction with the Serial Port
 */

#define SERIAL_PORT_IRQ_1         4

#define BASE_COM1						        0x3F8
#define BASE_COM2						        0x2F8

// UART Regs
#define RBR							            0 //Receiver Buffer Register
#define THR					                    0 //Transmitter Holding Register
#define IER						                1 //Interrupt Enable Register
#define	IIR				                        2 //Interupt Indentification Register
#define	FCR							            2 //FIFO Control Register
#define LCR							            3 //Line Control Register
#define MCR                                     4 //Modem Control Register
#define LSR								        5 //Line Status Register
#define MSR                                     6 //Modem Status Register
#define SR                                      7 //Scratchpad Register

#define DLL									    0 //Divisor Latch Least Significant Byte
#define DLM									    1 //Divisor Latch Most Significant Byte

// LCR Bits

#define BITS_PER_CHAR_5							0
#define BITS_PER_CHAR_6							BIT(0)
#define BITS_PER_CHAR_7							BIT(1)
#define BITS_PER_CHAR_8							(BIT(1) | BIT(0))

#define STOP_BITS_1								0
#define STOP_BITS_2								1

#define PARITY_NONE								0
#define PARITY_ODD								BIT(3)
#define PARITY_EVEN								(BIT(4) | BIT(3))
#define PARITY_1        						(BIT(5) | BIT(3))
#define PARITY_0			        			(BIT(5) | BIT(4) | BIT(3))
#define BREAK_CONTROL							BIT(6)
#define DLAB									BIT(7)

// LSR Bits
#define RECEIVER_READY							BIT(0)
#define OVERRUN_ERROR							BIT(1)
#define PARITY_ERROR							BIT(2)
#define FRAMING_ERROR							BIT(3)
#define TRANSMITTER_EMPTY						BIT(5)

// IER Bits
#define ENABLE_RECEIVED_DATA_INTERRUPT			BIT(0)
#define ENABLE_TRANSMITTER_EMPTY_INTERRUPT		BIT(1)
#define ENABLE_RECEIVER_LINE_STATUS_INTERRUPT	BIT(2)

// IIR Bits
#define INTERRUPT_STATUS_PENDING				0
#define INTERRUPT_STATUS_NOT_PENDING			BIT(0)

#define INTERRUPT_ORIGIN_MODEM					0
#define INTERRUPT_ORIGIN_TRANSMITTER_EMPTY		BIT(1)
#define INTERRUPT_ORIGIN_CHARACTER_TIMEOUT		BIT(3)
#define INTERRUPT_ORIGIN_RECEIVED_DATA			BIT(2)
#define INTERRUPT_ORIGIN_LINE_STATUS			(BIT(2) | BIT(1))

#define NO_FIFO									0
#define FIFO_UNUSABLE							BIT(7)
#define FIFO_ENABLED							(BIT(7) | BIT(6))

// FCR Bits

#define ENABLE_FIFO								BIT(0)
#define CLEAR_RECEIVE_FIFO						BIT(1)
#define CLEAR_TRANSMIT_FIFO						BIT(2)

#define FIFO_INTERRUPT_TRIGGER_LEVEL_1			0
#define FIFO_INTERRUPT_TRIGGER_LEVEL_4			BIT(6)
#define FIFO_INTERRUPT_TRIGGER_LEVEL_8			BIT(7)
#define FIFO_INTERRUPT_TRIGGER_LEVEL_14			(BIT(7) | BIT(6))

/**
 * @brief Subscribes to serial port
 * 
 * @param bit_no bit to set as serial port interrupt identifier
 * 
 * @return 0 if no errors occur and 1 otherwise
 */
int serial_port_subscribe_int(uint8_t *bit_no);

/**
 * @brief Unsubscribes to serial port
 * 
 * @return 0 if no errors occur and 1 otherwise
 */
int serial_port_unsubscribe_int();

/**
 * @brief Configures serial port to mode used in the project
 * 
 * @param port Port to change config (1/2)
 */
void serial_port_com1_config(int port);

/**
 * @brief Receives byte from the receiver buffer
 * 
 * @param bytes pointer to current serial port array
 * @param size size of current serial port array
 */
void serial_port_ih(char ** bytes, unsigned * size);

/**
 * @brief Checks if array has full serial port message
 * 
 * @param bytes pointer to current serial port array
 * @param size size of current serial port array
 * 
 * @return true if there's a full message, false otherwise
 */
bool got_full_message(char ** bytes, unsigned * size);

/**
 * @brief Clears serial port receiving buffer
 */
void clear_sp_buffer();

/**
 * @brief Sends a byte to transmitting buffer
 * 
 * @param data Byte to send
 */
void send_sp_byte(uint8_t data);

/**
 * @brief Sends a full message to transmitting buffer
 * 
 * @param message Message to send to buffer
 * @param size Size of message
 */
void send_sp_message(char * message, uint8_t size);

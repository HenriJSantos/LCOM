#pragma once

/** @defgroup Mouse Mouse
 * @{
 *
 * @brief Module for interaction with the mouse
 */

/**
 * @brief Subscribes mouse interrupts
 */
int(mouse_subscribe_int)(uint8_t *bit_no);

/**
 * @brief Unsubscribes mouse interrupts
 */
int(mouse_unsubscribe_int)();

/**
 * @brief Handles bytes provided by the mouse
 */
void(mouse_ih)();

/**
 * @brief Sends 0xD4 command to KBC and sends arg to mouse
 * 
 * @param arg Argument of the command to send
 * 
 * @return 0 if no errors occur and 1 otherwise
 */
int mouse_command(uint8_t arg);

/**
 * @brief Disables irq feed for mouse interrupts
 */
void disable_mouse_interrupts();

/**
 * @brief Enables irq feed for mouse interrupts
 */
void enable_mouse_interrupts();

/**
 * @brief Sets KBC command byte to its default
 * 
 * @return 0 if no errors occur and 1 otherwise
 */
int reset_kbc_cmd();

/**
 * @brief Takes an array of bytes constituting a packet and parses its information into a struct
 * 
 * @param packet Array that stores packet bytes
 * @param pp pointer to packet received
 */
void parse_packet(uint8_t packet[],struct packet *pp);

/**
 * @brief Takes bytes received by handler and sorts them into a 3 byte array (full packet)
 * 
 * @param packetCounter Counter that keeps track of which byte the handler is on
 * @param packetArray Array that stores packet bytes
 */
void handle_packet_byte(uint8_t * packetCounter, uint8_t packetArray[]);

/**
 * @brief Detects relevant mouse events
 * 
 * @param pp pointer to packet received
 * @param event pointer to event byte
 * 
 * @return Struct with mouse event specification
 */
struct mouse_ev detect_mouse_evt(struct packet *pp, uint8_t *event);

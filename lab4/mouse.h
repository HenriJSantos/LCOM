#pragma once

//Subscribes mouse interrupts
int(mouse_subscribe_int)(uint8_t *bit_no);

//Unsubscribes mouse interrupts
int(mouse_unsubscribe_int)();

//Handles bytes provided by the mouse
void(mouse_ih)();

//Sends 0xD4 command to KBC and sends arg to mouse
int mouse_command(uint8_t arg);

//Disables irq feed for mouse interrupts
void disable_mouse_interrupts();

//Enables irq feed for mouse interrupts
void enable_mouse_interrupts();

//Sets KBC command byte to its default
int reset_kbc_cmd();

//Takes an array of bytes constituting a packet and parses its information into a struct
void parse_packet(uint8_t packet[],struct packet *pp);

//Takes bytes received by handler and sorts them into a 3 byte array (full packet)
void handle_packet_byte(uint8_t * packetCounter, uint8_t packetArray[]);

//Detects relevant mouse events
struct mouse_ev detect_mouse_evt(struct packet *pp, uint8_t *event);

//State machine for inverted V drawing
typedef enum
{
  INIT,
  UP_RIGHT,
  VERTICE,
  DOWN_RIGHT,
  COMP
} state_t;

//Takes event and processes it according to current drawing state and type of event
void handle_mouse_evt(struct mouse_ev * mouse_event, state_t * gesture_state, uint8_t x_len, uint8_t tolerance);

#ifndef GAME_H
#define GAME_H

#include <lcom/lcf.h>
#include "UTIL.h"

/** @defgroup Game Game
 * @{
 *
 * @brief Main module for the game's processing
 */

#define TIMER_EVT_BIT   BIT(0)

#define KB_EVT_BIT      BIT(1)

#define MOUSE_EVT_BIT   BIT(2)

#define OUR_VIDEO_MODE  0x144

#define MAIN_CHAR_SPEED     5

#define BULLET_ARRAY_SIZE   10

#define ENEMY_ARRAY_SIZE    10

#define BULLET_COOLDOWN     20

#define ENEMY_SPEED         3

/**
 * @brief Basic cartisean coordinates struct
 */
typedef struct
{
  int x;    /*!< horizontal position */
  int y;    /*!< vertical position */
} coordinates;

/**
 * @brief Enumerated type state machine for identifying the game's current state
 */
typedef enum{
  MENU,                 /*!< Main menu state */
  GAME,                 /*!< Playing as singleplayer state */
  GAME_P1,              /*!< Playing as multiplayer player one state */
  GAME_P2,              /*!< Playing as multiplayer player two state */
  WAITING_FOR_P2,       /*!< Waiting for player two state */
  GAME_OVER,            /*!< End of current game state */
  END                   /*!< Project termination state */
} GAME_STATE_MACHINE;

/**
 * @brief Enumerated type for identifying timer event type
 */
enum TIMER_EVT_TYPE{
    UPDATE_FRAME,       /*!< Event that causes an update to the video memory */
    SPAWN_ENEMY,        /*!< Event that causes an enemy to spawn (in-game specific)*/
    INCREASE_SCORE      /*!< Event that causes a periodic increase in score (in-game specific)*/
};

/**
 * @brief Timer event struct that specifies type and enemy spawn coordinates
 */
struct
{
    enum TIMER_EVT_TYPE type;   /*!< Enum that specifies type of timer event */
    unsigned enemy_x;           /*!< If type is SPAWN_ENEMY, specifies horizontal coordinate to spawn enemy in*/
    unsigned enemy_y;           /*!< If type is SPAWN_ENEMY, specifies vertical coordinate to spawn enemy in*/
} timer_evt;

/**
 * @brief Enumerated type for identifying keyboard event type
 */
enum KB_EVT_TYPE{
    UP_START,       /*!< Event that causes main character to start moving up (in-game specific)*/
    UP_STOP,        /*!< Event that causes main character to stop moving up (in-game specific)*/
    LEFT_START,     /*!< Event that causes main character to start moving left (in-game specific)*/
    LEFT_STOP,      /*!< Event that causes main character to stop moving left (in-game specific)*/
    DOWN_START,     /*!< Event that causes main character to start moving down (in-game specific)*/
    DOWN_STOP,      /*!< Event that causes main character to stop moving down (in-game specific)*/
    RIGHT_START,    /*!< Event that causes main character to start moving right (in-game specific)*/
    RIGHT_STOP,     /*!< Event that causes main character to stop moving right (in-game specific)*/
    ESCAPE          /*!< Event that causes an escape from current state*/
};

/**
 * @brief Keyboard event struct that specifies type
 */
struct
{
    enum KB_EVT_TYPE type;  /*!< Enum that specifies type of keyboard event */
} kb_evt;

/**
 * @brief Enumerated type for identifying mouse event type
 */
enum MOUSE_EVT_TYPE{
    MOVE_CROSSHAIR, /*!< Event that causes mouse to move a certain ammount specified in event struct*/
    SET_CROSSHAIR,  /*!< Event that causes mouse to move to coordinates specified in event struct*/
    CLICK           /*!< Event that causes an interaction at current mouse position*/
};

/**
 * @brief Mouse event struct that specifies type
 */
struct 
{
    enum MOUSE_EVT_TYPE type;   /*!< Enum that specifies type of keyboard event */
    int delta_x;                /*!< Horizontal mouse coordinate to increment/set depending on event type */
    int delta_y;                /*!< Vertical mouse coordinate to increment/set depending on event type */
} mouse_evt;

/**
 * @brief Byte that contains bit flags for different device events
 */
uint8_t event_byte;

/**
 * @brief Basic vector direction 
 * 
 * Returns a vector with direction orig->dest and module "speed".
 * 
 * @param orig Original point
 * @param dest Destination point
 * @param speed Module of directional vector
 * 
 * @return vector with direction orig->dest and module "speed".
 */
coordinates getTrajectory(coordinates orig, coordinates dest, int speed);

/**
 * @brief Checks if a color is from a bullet
 *
 * @param Color to check
 * 
 * @return true if it's a bullet color, false otherwise.
 */
bool isBulletColor(uint32_t color);

/**
 * @brief Project's interrupt handler
 * 
 * Receives interrupts from devices and raises relevant event flags.
 * Also calls event handler any time there is an event raised.
 * 
 */
void interrupt_handler();

/**
 * @brief Project's event handler
 * 
 * Parses event byte and according to event received, acts based on current game state
 * 
 */
void event_handler();

/**
 * @brief Project's start-up function
 * 
 * Loads all necessary bitmaps and starts up with the menu displayed
 * 
 */
void init_project();

/**
 * @brief Game's start-up function
 * 
 * Sets all necessary game variables to default
 * 
 */
void init_game();

/**
 * @brief Game's end function
 * 
 * Prints score and frees all sprites
 * 
 */
void end_game();

/**
 * @brief Project's end function
 * 
 * Frees all bitmap memory
 * 
 */
void end_project();

/** @} end of game */

#endif

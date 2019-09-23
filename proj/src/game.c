#include <math.h>

#include "game.h"
#include "bmp.h"
#include "keyboard.h"
#include "video.h"
#include "i8042.h"
#include "mouse.h"
#include "sprites.h"
#include "RTC.h"
#include "serial_port.h"
#include "text.h"

//General device data
extern uint32_t scancode;
extern uint16_t timer_counter;
extern bool mouse_ih_return;
extern uint8_t irq_set_kb, irq_set_timer, irq_set_mouse, irq_set_com1;
static vbe_mode_info_t info;

//Game state machine
static GAME_STATE_MACHINE GAME_STATE = MENU;

//Bitmaps
static Bitmap *game_day_background;
static Bitmap *game_night_background;
static Bitmap *menu_background;
static Bitmap *waiting_for_player_2_background;
static Bitmap *game_over_background;
static Bitmap *mainCharBmp1;
static Bitmap *mainCharBmp2;
static Bitmap *crosshair;
static Bitmap *enemy1Bmp, *enemy2Bmp, *enemy3Bmp;
static Bitmap *bulletBmp;

//Mouse coordinates
static int mouse_x = 512, mouse_y = 384;

//Main character movement booleans
static bool movUp = false, movLeft = false, movDown = false, movRight = false;
//Main character firing cooldown
static uint8_t bullet_cooldown;
//Ticks between enemy spawns
static uint8_t enemy_delay = 150;
//Score
static unsigned int score = 0;

//Sprites
static Animated_Sprite *mainChar;
static Sprite *bullets[BULLET_ARRAY_SIZE];
static Animated_Sprite *enemies[ENEMY_ARRAY_SIZE];

//Multiplayer
static uint8_t player_no = 1;

void init_game();
void end_game();
void init_project();
void end_project();

coordinates getTrajectory(coordinates orig, coordinates dest, int speed)
{
  double vect_x = dest.x - orig.x;
  double vect_y = dest.y - orig.y;
  double len = sqrt(vect_x * vect_x + vect_y * vect_y);
  vect_x /= len;
  vect_y /= len;
  vect_x = round(vect_x * speed);
  vect_y = round(vect_y * speed);
  coordinates result = {.x = vect_x, .y = vect_y};
  return result;
}

bool isBulletColor(uint32_t color)
{
  return (color == 0xfff93527 || color == 0xfff95a51 || color == 0xfffa9f96 || color == 0xfffdbab2 || color == 0xffffe8e5);
}

void interrupt_handler()
{
  int ipc_status, r;
  message msg;
  uint8_t packetCounter = 0;
  uint8_t packetArray[3];
  struct packet pp;
  char * sp_bytes = NULL;
  unsigned int sp_message_size = 0;

  while (GAME_STATE != END)
  {
    // Get a request message
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status))
    { // Received notification
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE: // Hardware interrupt notification
        if (msg.m_notify.interrupts & BIT(irq_set_kb))
        {
          kbc_ih();
          if(GAME_STATE != GAME_P2 || scancode == ESC_BREAKCODE)
          {
            switch (scancode)
            {
            case ESC_BREAKCODE:
              kb_evt.type = ESCAPE;
              event_byte |= KB_EVT_BIT;
              break;
            case W_MAKECODE:
              kb_evt.type = UP_START;
              event_byte |= KB_EVT_BIT;
              break;
            case W_BREAKCODE:
              kb_evt.type = UP_STOP;
              event_byte |= KB_EVT_BIT;
              break;
            case A_MAKECODE:
              kb_evt.type = LEFT_START;
              event_byte |= KB_EVT_BIT;
              break;
            case A_BREAKCODE:
              kb_evt.type = LEFT_STOP;
              event_byte |= KB_EVT_BIT;
              break;
            case S_MAKECODE:
              kb_evt.type = DOWN_START;
              event_byte |= KB_EVT_BIT;
              break;
            case S_BREAKCODE:
              kb_evt.type = DOWN_STOP;
              event_byte |= KB_EVT_BIT;
              break;
            case D_MAKECODE:
              kb_evt.type = RIGHT_START;
              event_byte |= KB_EVT_BIT;
              break;
            case D_BREAKCODE:
              kb_evt.type = RIGHT_STOP;
              event_byte |= KB_EVT_BIT;
              break;
            }
          }
          if(GAME_STATE == GAME_P1 || (GAME_STATE == GAME_P2 && scancode == ESC_BREAKCODE))
          {
            char message[2] = "K";
            message[1] = kb_evt.type;
            send_sp_message(message,2);
          }
        }
        if (msg.m_notify.interrupts & BIT(irq_set_timer))
        {
          timer_int_handler();
          if (timer_counter % (sys_hz() / 60) == 0)
          {
            event_byte |= TIMER_EVT_BIT;
            timer_evt.type = UPDATE_FRAME;
          }
          if (timer_counter % 5 == 0 && (GAME_STATE == GAME || GAME_STATE == GAME_P1 || GAME_STATE == GAME_P2))
          {
            event_byte |= TIMER_EVT_BIT;
            timer_evt.type = INCREASE_SCORE;
          }
          if (timer_counter % enemy_delay == 0 && GAME_STATE != GAME_P2)
          {
            event_byte |= TIMER_EVT_BIT;
            timer_evt.type = SPAWN_ENEMY;
            timer_evt.enemy_x = rand() % 2;
            timer_evt.enemy_y = rand() % 2;
            enemy_delay -= enemy_delay*0.017;
            if (enemy_delay < 60)
              enemy_delay = 60;
            if (GAME_STATE == GAME_P1)
            {
              char message[3] = "T";
              message[1] = timer_evt.enemy_x;
              message[2] = timer_evt.enemy_y;
              send_sp_message(message,3);
            }
          }
        }
        if (msg.m_notify.interrupts & BIT(irq_set_mouse))
        {
          //Read packet byte
          mouse_ih();
          if (mouse_ih_return != 0)
            continue;
          //Sort packet byte into array
          handle_packet_byte(&packetCounter, packetArray);

          if (packetCounter == 3)
          {
            packetCounter = 0;
            parse_packet(packetArray, &pp);

            //Previous packet received for comparing with current packet
            static struct packet prevPacket;

            //Press LB
            if (prevPacket.lb == 0 && pp.lb == 1 && pp.rb == 0 && pp.mb == 0)
            {
              if(GAME_STATE != GAME_P1)
              {
                event_byte |= MOUSE_EVT_BIT;
                mouse_evt.type = CLICK;
                mouse_evt.delta_x = 0;
                mouse_evt.delta_y = 0;
              }
              if(GAME_STATE == GAME_P2)
              {
                char message[6] = "M\1\0\0\0\0";
                send_sp_message(message,6);
              }
            }
            //Move mouse
            else if (pp.delta_x != 0 || pp.delta_y != 0)
            {
              if(GAME_STATE != GAME_P1)
              {
                event_byte |= MOUSE_EVT_BIT;
                mouse_evt.type = MOVE_CROSSHAIR;
                mouse_evt.delta_x = pp.delta_x;
                mouse_evt.delta_y = pp.delta_y;
              }
              if(GAME_STATE == GAME_P2)
              {
                char message[6] = "M\0";
                message[2] = (mouse_x + pp.delta_x) / 16;
                message[3] = (mouse_x + pp.delta_x) % 16;
                message[4] = (mouse_y + pp.delta_y) / 16;
                message[5] = (mouse_y + pp.delta_y) % 16;
                send_sp_message(message,6);
              }
            }
            //Set previous packet as current packet for next iteration
            prevPacket = pp;
          }
        }
        if (msg.m_notify.interrupts & BIT(irq_set_com1))
        { 
          serial_port_ih(&sp_bytes, &sp_message_size);
          if(got_full_message(&sp_bytes, &sp_message_size))
          {
            switch(sp_bytes[0])
            {
            case 'T':
              event_byte |= TIMER_EVT_BIT;
              timer_evt.type = SPAWN_ENEMY;
              timer_evt.enemy_x = sp_bytes[1];
              timer_evt.enemy_y = sp_bytes[2];
              break;
            case 'P':
              if(GAME_STATE == WAITING_FOR_P2)
              {
                GAME_STATE = GAME_P1;
                if(get_hour() > 7 && get_hour() < 21)
                {
                  setBackground(game_day_background);
                  drawBitmap(game_day_background, 0, 0, false);
                }
                else
                {
                  setBackground(game_night_background);
                  drawBitmap(game_night_background, 0, 0, false);
                }
              }
              else
                player_no = 2;
              break;
            case 'M':
              event_byte |= MOUSE_EVT_BIT;
              switch(sp_bytes[1])
              {
              case 0:
                mouse_evt.type = SET_CROSSHAIR;
                mouse_evt.delta_x = sp_bytes[2]*16+sp_bytes[3];
                mouse_evt.delta_y = sp_bytes[4]*16+sp_bytes[5];
                break;
              case 1:
                mouse_evt.type = CLICK;
                mouse_evt.delta_x = 0;
                mouse_evt.delta_y = 0;
                break;
              }
              break;
            case 'K':
              event_byte |= KB_EVT_BIT;
              kb_evt.type = sp_bytes[1];
              break;
            }
            free(sp_bytes);
            sp_bytes = NULL;
            sp_message_size = 0;
          }
        }
        break;
      default:
        break;
      }
      if (event_byte != 0)
      {
        event_handler();
      }
    }
  }
}

void event_handler()
{
  if (event_byte & TIMER_EVT_BIT)
  {
    event_byte ^= TIMER_EVT_BIT;
    switch (GAME_STATE)
    {
    case GAME_P1:
    case GAME_P2:
    case GAME:
      if (timer_evt.type == INCREASE_SCORE)
      {
        hide_int(score, 332, 0);
        score++;
        print_int(score, 332, 0);
      }
      if (timer_evt.type == UPDATE_FRAME || timer_evt.type == INCREASE_SCORE)
      {
        if (bullet_cooldown > 0)
          bullet_cooldown--;

        hide_animated_sprite(mainChar);
        hideBitmap(crosshair, mouse_x, mouse_y);

        for (unsigned int i = 0; i < BULLET_ARRAY_SIZE; i++)
        {
          if (bullets[i] != NULL)
            hideBitmap(bullets[i]->bmp, bullets[i]->x, bullets[i]->y);
        }

        for (unsigned int i = 0; i < ENEMY_ARRAY_SIZE; i++)
        {
          if (enemies[i] != NULL)
            hide_animated_sprite(enemies[i]);
        }

        if (movUp && mainChar->y - mainChar->yspeed > 0)
          mainChar->y -= mainChar->yspeed;
        if (movDown && mainChar->y + mainChar->yspeed + mainChar->bmps[0]->bitmapInfoHeader.height < info.YResolution)
          mainChar->y += mainChar->yspeed;
        if (movLeft && mainChar->x - mainChar->xspeed > 0)
          mainChar->x -= mainChar->xspeed;
        if (movRight && mainChar->x + mainChar->xspeed + mainChar->bmps[0]->bitmapInfoHeader.width < info.XResolution)
          mainChar->x += mainChar->xspeed;
        uint32_t pixelColorReturn;

        for (unsigned int i = 0; i < BULLET_ARRAY_SIZE; i++)
        {
          if (bullets[i] != NULL)
            if (move_sprite(bullets[i], false))
            {
              hide_sprite(bullets[i]);
              free(bullets[i]);
              bullets[i] = NULL;
            }
        }

        for (unsigned int i = 0; i < ENEMY_ARRAY_SIZE; i++)
        {
          if (enemies[i] != NULL)
          {
            coordinates orig = {.x = enemies[i]->x + enemies[i]->width / 2, .y = enemies[i]->y + enemies[i]->height / 2};
            coordinates dest = {.x = mainChar->x + mainChar->width / 2, .y = mainChar->y + mainChar->height / 2};
            coordinates vect = getTrajectory(orig, dest, ENEMY_SPEED);
            enemies[i]->xspeed = vect.x;
            enemies[i]->yspeed = vect.y;
            if ((pixelColorReturn = move_animated_sprite(enemies[i], true)))
            {
              if (pixelColorReturn == 0xfff93527)
              {
                hide_int(score, 332, 0);
                score += 25;
                print_int(score, 332, 0);
                hide_animated_sprite(enemies[i]);
                free(enemies[i]);
                enemies[i] = NULL;
                for (unsigned int i = 0; i < BULLET_ARRAY_SIZE; i++)
                {
                  if (bullets[i] != NULL)
                    hideBitmap(bullets[i]->bmp, bullets[i]->x, bullets[i]->y);
                  free(bullets[i]);
                  bullets[i] = NULL;
                }
              }
            }
          }
        }
        if ((pixelColorReturn = draw_animated_sprite(mainChar, true)))
        {
          if (isBulletColor(pixelColorReturn))
          {
            draw_animated_sprite(mainChar, false);
          }
          else
          {
            //Dead
            draw_animated_sprite(mainChar, false);
            end_game();
            GAME_STATE = GAME_OVER;
            setBackground(game_over_background);
            drawBitmap(game_over_background,0,0,false);
          }
        }

        drawBitmap(crosshair, mouse_x, mouse_y, false);
        updateFrame();
      }
      if (timer_evt.type == SPAWN_ENEMY)
      {
        Animated_Sprite * enemy = create_animated_sprite(timer_evt.enemy_x * 928, 100 + timer_evt.enemy_y % 2 * 570,2,2,16,enemy1Bmp, enemy1Bmp, enemy1Bmp, enemy1Bmp, enemy2Bmp, enemy2Bmp, enemy2Bmp, enemy2Bmp, enemy3Bmp, enemy3Bmp, enemy3Bmp, enemy3Bmp, enemy2Bmp, enemy2Bmp, enemy2Bmp, enemy2Bmp);
        for (unsigned int i = 0; i < ENEMY_ARRAY_SIZE; i++)
        {
          if(enemies[i] == NULL)
          {
            enemies[i] = enemy;
            break;
          }
          else if (i==ENEMY_ARRAY_SIZE)
            free(enemy);
        }
      }
      break;
    case MENU:
      break;
    case GAME_OVER:
      if (timer_evt.type == UPDATE_FRAME)
      {
        print_int(score,512,210);
        updateFrame();
      }
      break;
    default:
      break;
    }
  }

  if (event_byte & KB_EVT_BIT)
  {
    event_byte ^= KB_EVT_BIT;
    switch (GAME_STATE)
    {
    case MENU:
      switch(kb_evt.type)
      {
      case ESCAPE:
        end_game();
        end_project();
        GAME_STATE = END;
        break;
      default:
        break;
      }
      break;
    case GAME_P1:
    case GAME_P2:
    case GAME:
      switch (kb_evt.type)
      {
      case ESCAPE:
        end_game();
        GAME_STATE = MENU;
        setBackground(menu_background);
        drawBitmap(menu_background,0,0,false);
        updateFrame();
        break;
      case UP_START:
        movUp = true;
        break;
      case UP_STOP:
        movUp = false;
        break;
      case LEFT_START:
        movLeft = true;
        break;
      case LEFT_STOP:
        movLeft = false;
        break;
      case DOWN_START:
        movDown = true;
        break;
      case DOWN_STOP:
        movDown = false;
        break;
      case RIGHT_START:
        movRight = true;
        break;
      case RIGHT_STOP:
        movRight = false;
        break;
      default:
        break;
      }
      break;
    case GAME_OVER:
      switch(kb_evt.type)
      {
      case ESCAPE:
        end_game();
        end_project();
        GAME_STATE = END;
        break;
      default:
        break;
      }
      break;
    case WAITING_FOR_P2:
      switch (kb_evt.type)
      {
      case ESCAPE:
        end_game();
        GAME_STATE = MENU;
        setBackground(menu_background);
        drawBitmap(menu_background,0,0,false);
        updateFrame();
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
  }

  if (event_byte & MOUSE_EVT_BIT)
  {
    event_byte ^= MOUSE_EVT_BIT;
    switch (GAME_STATE)
    {
    case MENU:
      if (mouse_evt.type == CLICK)
      {
        if (sqrt(pow(mouse_x-510,2)+pow(mouse_y-465,2))<115)
        {
          GAME_STATE = GAME;
          init_game();
          if(get_hour() > 7 && get_hour() < 21)
          {
            setBackground(game_day_background);
            drawBitmap(game_day_background, 0, 0, false);
          }
          else
          {
            setBackground(game_night_background);
            drawBitmap(game_night_background, 0, 0, false);
          }
          updateFrame();
        }
        if (sqrt(pow(mouse_x-300,2)+pow(mouse_y-595,2))<115)
        {
          if (player_no == 1)
          {
            GAME_STATE = WAITING_FOR_P2;
            setBackground(waiting_for_player_2_background);
            drawBitmap(waiting_for_player_2_background, 0, 0, false);
            updateFrame();
          }
          else if (player_no == 2)
          {
            GAME_STATE = GAME_P2;
            if(get_hour() > 7 && get_hour() < 21)
            {
              setBackground(game_day_background);
              drawBitmap(game_day_background, 0, 0, false);
            }
            else
            {
              setBackground(game_night_background);
              drawBitmap(game_night_background, 0, 0, false);
            }
          }
          init_game();
          updateFrame();
          send_sp_message("P",1);
        }
        if (sqrt(pow(mouse_x-725,2)+pow(mouse_y-600,2))<115)
        {
          GAME_STATE = END;
          end_project();
        }
      }
      if (mouse_evt.type == MOVE_CROSSHAIR)
      {
        hideBitmap(crosshair, mouse_x, mouse_y);
        if (mouse_x + mouse_evt.delta_x < 0)
          mouse_x = 0;
        else if (mouse_x + mouse_evt.delta_x + crosshair->bitmapInfoHeader.width >= info.XResolution)
          mouse_x = info.XResolution - crosshair->bitmapInfoHeader.width;
        else
          mouse_x += mouse_evt.delta_x;
        if (mouse_y - mouse_evt.delta_y < 0)
          mouse_y = 0;
        else if (mouse_y - mouse_evt.delta_y + crosshair->bitmapInfoHeader.height >= info.YResolution)
          mouse_y = info.YResolution - crosshair->bitmapInfoHeader.height;
        else
          mouse_y -= mouse_evt.delta_y;
        drawBitmap(crosshair, mouse_x, mouse_y, false);
        updateFrame();
      }
      break;
    case GAME_P1:
    case GAME_P2:
    case GAME:
      if (mouse_evt.type == CLICK && bullet_cooldown == 0)
      {
        bullet_cooldown = BULLET_COOLDOWN;
        coordinates orig = {.x = mainChar->x + mainChar->width / 2, .y = mainChar->y + mainChar->height / 2};
        coordinates dest = {.x = mouse_x, .y = mouse_y};
        coordinates vect = getTrajectory(orig, dest, 10);
        Sprite *bullet = create_sprite(bulletBmp, mainChar->x + mainChar->width / 2, mainChar->y + mainChar->height / 2, vect.x, vect.y);
        for (unsigned int i = 0; i < BULLET_ARRAY_SIZE; i++)
        {
          if (bullets[i] == NULL)
          {
            bullets[i] = bullet;
            break;
          }
        }
      }
      
      if (mouse_evt.type == MOVE_CROSSHAIR)
      {
        hideBitmap(crosshair, mouse_x, mouse_y);

        if (mouse_x + mouse_evt.delta_x < 0)
          mouse_x = 0;
        else if (mouse_x + mouse_evt.delta_x + crosshair->bitmapInfoHeader.width >= info.XResolution)
          mouse_x = info.XResolution - crosshair->bitmapInfoHeader.width;
        else
          mouse_x += mouse_evt.delta_x;

        if (mouse_y - mouse_evt.delta_y < 100)
          mouse_y = 100;
        else if (mouse_y - mouse_evt.delta_y + crosshair->bitmapInfoHeader.height >= info.YResolution)
          mouse_y = info.YResolution - crosshair->bitmapInfoHeader.height;
        else
          mouse_y -= mouse_evt.delta_y;

        drawBitmap(crosshair, mouse_x, mouse_y, false);
        updateFrame();
      }

      if (mouse_evt.type == SET_CROSSHAIR)
      {
        hideBitmap(crosshair, mouse_x, mouse_y);

        if(mouse_evt.delta_x < 0)
          mouse_x = 0;
        else if (mouse_evt.delta_x + crosshair->bitmapInfoHeader.width >= info.XResolution)
          mouse_x = info.XResolution - crosshair->bitmapInfoHeader.width;
        else
          mouse_x = mouse_evt.delta_x;

        if(mouse_evt.delta_y < 100)
          mouse_y = 100;
        else if (mouse_evt.delta_y + crosshair->bitmapInfoHeader.height >= info.YResolution)
          mouse_y = info.YResolution - crosshair->bitmapInfoHeader.height;
        else
          mouse_y = mouse_evt.delta_y;

        drawBitmap(crosshair, mouse_x, mouse_y, false);
        updateFrame();
      }
      break;
    case GAME_OVER:
    if (mouse_evt.type == CLICK)
      {
        if (sqrt(pow(mouse_x-145,2)+pow(mouse_y-630,2))<115)
        {
          GAME_STATE = GAME;
          init_game();
          if(get_hour() > 7 && get_hour() < 21)
          {
            setBackground(game_day_background);
            drawBitmap(game_day_background, 0, 0, false);
          }
          else
          {
            setBackground(game_night_background);
            drawBitmap(game_night_background, 0, 0, false);
          }
          updateFrame();
        }
        if (sqrt(pow(mouse_x-895,2)+pow(mouse_y-635,2))<115)
        {
          GAME_STATE = MENU;
          setBackground(menu_background);
          drawBitmap(menu_background, 0, 0, false);
          updateFrame();
        }
      }
      if (mouse_evt.type == MOVE_CROSSHAIR)
      {
        hideBitmap(crosshair, mouse_x, mouse_y);
        if (mouse_x + mouse_evt.delta_x > 0 && mouse_x + mouse_evt.delta_x + crosshair->bitmapInfoHeader.width < info.XResolution)
          mouse_x += mouse_evt.delta_x;
        else if (mouse_x + mouse_evt.delta_x < 0)
          mouse_x = 0;
        else if (mouse_x + mouse_evt.delta_x + crosshair->bitmapInfoHeader.width >= info.XResolution)
          mouse_x = info.XResolution - crosshair->bitmapInfoHeader.width;
        if (mouse_y - mouse_evt.delta_y > 100 && mouse_y - mouse_evt.delta_y + crosshair->bitmapInfoHeader.height < info.YResolution)
          mouse_y -= mouse_evt.delta_y;
        else if (mouse_y - mouse_evt.delta_y < 100)
          mouse_y = 100;
        else if (mouse_y - mouse_evt.delta_y + crosshair->bitmapInfoHeader.height >= info.YResolution)
          mouse_y = info.YResolution - crosshair->bitmapInfoHeader.height;
        drawBitmap(crosshair, mouse_x, mouse_y, false);
        updateFrame();
      }
      break;
    default:
      break;
    }
  }
}

void init_game()
{
  enemy_delay = 150;

  score = 0;

  movUp = false;
  movLeft = false;
  movRight = false;
  movDown = false;

  mainChar = create_animated_sprite(info.XResolution / 2 - mainCharBmp1->bitmapInfoHeader.width / 2, info.YResolution / 2 - mainCharBmp1->bitmapInfoHeader.height / 2, MAIN_CHAR_SPEED, MAIN_CHAR_SPEED, 2, mainCharBmp1, mainCharBmp2);

  mainChar->x = info.XResolution / 2 - mainCharBmp1->bitmapInfoHeader.width / 2;
  mainChar->y = info.YResolution / 2 - mainCharBmp1->bitmapInfoHeader.height / 2;

  mouse_x = info.XResolution / 2 - crosshair->bitmapInfoHeader.width / 2;
  mouse_y = info.YResolution / 2 - crosshair->bitmapInfoHeader.height / 2;

  for (unsigned int i = 0; i < BULLET_ARRAY_SIZE; i++)
    bullets[i] = NULL;

  for (unsigned int i = 0; i < ENEMY_ARRAY_SIZE; i++)
    enemies[i] = NULL;
}

void init_project()
{
  game_day_background = loadBitmap("/home/lcom/labs/proj/bmp/game_day_background.bmp");
  game_night_background = loadBitmap("/home/lcom/labs/proj/bmp/game_night_background.bmp");
  menu_background = loadBitmap("/home/lcom/labs/proj/bmp/menu_background.bmp");
  waiting_for_player_2_background = loadBitmap("/home/lcom/labs/proj/bmp/waiting_for_player_2_background.bmp");
  game_over_background = loadBitmap("/home/lcom/labs/proj/bmp/game_over_background.bmp");
  crosshair = loadBitmap("/home/lcom/labs/proj/bmp/crosshair.bmp");
  mainCharBmp1 = loadBitmap("/home/lcom/labs/proj/bmp/pikachu1.bmp");
  mainCharBmp2 = loadBitmap("/home/lcom/labs/proj/bmp/pikachu2.bmp");
  enemy1Bmp = loadBitmap("/home/lcom/labs/proj/bmp/enemy1.bmp");
  enemy2Bmp = loadBitmap("/home/lcom/labs/proj/bmp/enemy2.bmp");
  enemy3Bmp = loadBitmap("/home/lcom/labs/proj/bmp/enemy3.bmp");
  bulletBmp = loadBitmap("/home/lcom/labs/proj/bmp/bullet.bmp");

  init_numbers();

  parse_vbe_mode(OUR_VIDEO_MODE, &info);

  setBackground(menu_background);
  drawBitmap(menu_background, 0, 0, false);
  drawBitmap(crosshair, mouse_x, mouse_y, false);
  updateFrame();
}

void end_game()
{
  print_score(score);
  delete_animated_sprite(mainChar);
  for (unsigned int i = 0; i < ENEMY_ARRAY_SIZE; i++)
  {
    if(enemies[i] != NULL)
    {
      delete_animated_sprite(enemies[i]);
      enemies[i] = NULL;
    }
  }
  for (unsigned int i = 0; i < BULLET_ARRAY_SIZE; i++)
  {
    if(bullets[i] != NULL)
    {
      delete_sprite(bullets[i]);
      bullets[i] = NULL;
    }
  }
  if(GAME_STATE == GAME_P1 || GAME_STATE == GAME_P2)
  {
    player_no = 1;
    clear_sp_buffer();
  }
}

void end_project()
{
  deleteBitmap(mainCharBmp1);
  deleteBitmap(mainCharBmp2);
  deleteBitmap(game_day_background);
  deleteBitmap(game_night_background);
  deleteBitmap(menu_background);
  deleteBitmap(waiting_for_player_2_background);
  deleteBitmap(game_over_background);
  deleteBitmap(crosshair);
  deleteBitmap(enemy1Bmp);
  deleteBitmap(enemy2Bmp);
  deleteBitmap(enemy3Bmp);
  deleteBitmap(bulletBmp);
  free_numbers();
}

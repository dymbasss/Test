#ifndef __ZB_HEADER_FOR_BUTTON_H
#define __ZB_HEADER_FOR_BUTTON_H

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

// Button_left - Button_right
#define B_LEFT GPIO_Pin_0
#define B_RIGHT GPIO_Pin_1

void left_button_hendler(zb_uint8_t param);
void right_button_hendler(zb_uint8_t param);
void double_click_hendler(zb_uint8_t param);
void set_double_click_hendler(zb_callback_t func);
void set_left_button_hendler(zb_callback_t func);
void set_right_button_hendler(zb_callback_t func);
void zr_send_led_command(zb_uint8_t param);
void init_button(void);

typedef enum
  {
    LED_COMMAND_TOGGLE = 0,
    LED_COMMAND_STEP_UP,
    LED_COMMAND_CHANGE_COLOR
  } led_command;

#endif // !ZB_HEADERFOR_LED_H

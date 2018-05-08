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

void set_double_click_handler(zb_callback_t func);
void set_left_button_handler(zb_callback_t func);
void set_right_button_handler(zb_callback_t func);
void init_timer(void);
void init_button(void);

typedef enum
  {
    LED_COMMAND_TOGGLE = 0,
    LED_COMMAND_STEP_UP,
    LED_COMMAND_CHANGE_COLOR
  } led_command;

typedef enum
  {
    TIM_CMD_DISABLE = 0,
    TIM_CMD_ENABLE
  } tim_cmd;

#endif // !ZB_HEADERFOR_LED_H

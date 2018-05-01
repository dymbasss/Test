#ifndef __ZB_HEADER_FOR_LED_H
#define __ZB_HEADER_FOR_LED_H 

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

// Button_left - Button_right
#define B_LEFT GPIO_Pin_0
#define B_RIGHT GPIO_Pin_1

// Pin_6 - Pin_7 - Pin_8
#define L_PIN_RED GPIO_Pin_6
#define L_PIN_GREEN GPIO_Pin_7
#define L_PIN_BLUE GPIO_Pin_8

#define PWM_FREQ 1000
#define PERIOD (zb_uint16_t)(84000000 / PWM_FREQ - 1) // 1 kHz -> 8399 = (timer_clock / PWM_frequency) - 1

#define FIRST_BYTE 0

typedef enum
  {
    LED_COMMAND_TOGGLE = 0,
    LED_COMMAND_STEP_UP,
    LED_COMMAND_CHANGE_COLOR
  } led_command;

typedef enum
  {
    COLOR_RED = 0,
    COLOR_GREEN,
    COLOR_BLUE
  } led_color;

void init_button(void);

void init_led(void);

void init_timer_pwm(void);

#endif // !ZB_HEADERFOR_LED_H

#ifndef __ZB_HEADER_FOR_LED_H
#define __ZB_HEADER_FOR_LED_H 

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zb_secur.h"
#include "zb_secur_api.h"

typedef enum command
  {
    LED_COMMAND_ON = 0,
    LED_COMMAND_OFF,
    LED_COMMAND_TOGGLE,
    LED_COMMAND_SET_TO_LEVEL,
    LED_COMMAND_STEP_UP,
    LED_COMMAND_STEP_DOWN
  } led_command;
  
#endif // !ZB_HEADERFOR_LED_H

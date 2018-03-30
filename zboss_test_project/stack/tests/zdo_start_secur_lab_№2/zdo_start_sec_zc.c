/***************************************************************************
*                      ZBOSS ZigBee Pro 2007 stack                         *
*                                                                          *
*          Copyright (c) 2012 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*          Copyright (c) 2011 ClarIDy Solutions, Inc., Taipei, Taiwan.     *
*                       http://www.claridy.com/                            *
*                                                                          *
*          Copyright (c) 2011 Uniband Electronic Corporation (UBEC),       *
*                             Hsinchu, Taiwan.                             *
*                       http://www.ubec.com.tw/                            *
*                                                                          *
*          Copyright (c) 2011 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*                                                                          *
*                                                                          *
* ZigBee Pro 2007 stack, also known as ZBOSS (R) ZB stack is available     *
* under either the terms of the Commercial License or the GNU General      *
* Public License version 2.0.  As a recipient of ZigBee Pro 2007 stack, you*
* may choose which license to receive this code under (except as noted in  *
* per-module LICENSE files).                                               *
*                                                                          *
* ZBOSS is a registered trademark of DSR Corporation AKA Data Storage      *
* Research LLC.                                                            *
*                                                                          *
* GNU General Public License Usage                                         *
* This file may be used under the terms of the GNU General Public License  *
* version 2.0 as published by the Free Software Foundation and appearing   *
* in the file LICENSE.GPL included in the packaging of this file.  Please  *
* review the following information to ensure the GNU General Public        *
* License version 2.0 requirements will be met:                            *
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.                   *
*                                                                          *
* Commercial Usage                                                         *
* Licensees holding valid ClarIDy/UBEC/DSR Commercial licenses may use     *
* this file in accordance with the ClarIDy/UBEC/DSR Commercial License     *
* Agreement provided with the Software or, alternatively, in accordance    *
* with the terms contained in a written agreement between you and          *
* ClarIDy/UBEC/DSR.                                                        *
*                                                                          *
****************************************************************************
PURPOSE: Test for ZC application written using ZDO.
*/

#include "zdo_header_for_led.h"

#ifndef ZB_COORDINATOR_ROLE
#error Coordinator role is not compiled!
#endif

#ifndef ZB_SECURITY
#error Define ZB_SECURITY
#endif

zb_ieee_addr_t g_ieee_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x08};
zb_uint8_t g_key[16] = { 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0, 0, 0, 0, 0, 0, 0, 0};

void zr_led_command(zb_uint8_t *ptr);
void data_indication(zb_uint8_t param) ZB_CALLBACK;

#define FIRST_BYTE 0
#define SECOND_BYTE 1
#define LED_BRIGHTNESS_STEP 50

void zr_on();
void zr_off();
void zr_toggle();
void zr_set_to_level(zb_uint8_t *ptr);
void zr_step_up();
void zr_step_down();

zb_bool_t led_state;
zb_uint8_t led_brightness;

MAIN()
{
  ARGV_UNUSED;

#ifndef KEIL
  if ( argc < 3 )
  {
    printf("%s <read pipe path> <write pipe path>\n", argv[0]);
    return 0;
  }
#endif

  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_zc", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zc", "1", "1");
#endif
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_ieee_addr);
  /* let's always be coordinator */
  ZB_AIB().aps_designated_coordinator = 1;
  zb_secur_setup_preconfigured_key(g_key, 0);

  if (zdo_dev_start() != RET_OK)
  {
    TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
  }
  else
  {
    zdo_main_loop();
  }

  TRACE_DEINIT();

  MAIN_RETURN(0);
}

void zb_zdo_startup_complete(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  TRACE_MSG(TRACE_APS3, ">>zb_zdo_startup_complete status %d",
	    (FMT__D, (int)buf->u.hdr.status));
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
    zb_af_set_data_indication(data_indication);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device start FAILED status %d",(FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}

void data_indication(zb_uint8_t param)
{
  zb_uint8_t *ptr;
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  /* Remove APS header from the packet */
  ZB_APS_HDR_CUT_P(asdu, ptr);

  TRACE_MSG(TRACE_APS2, "data_indication: packet %p len %d handle 0x%x", (FMT__P_D_D,
                         asdu, (int)ZB_BUF_LEN(asdu), asdu->u.hdr.status));

  zr_led_command(ptr);
  zb_free_buf(asdu);   
}

void zr_led_command(zb_uint8_t *ptr)
{
  switch(ptr[FIRST_BYTE])
    {
    case LED_COMMAND_ON:
      zr_on();
      break;
    case LED_COMMAND_OFF:
      zr_off();
      break;
    case LED_COMMAND_TOGGLE:
      zr_toggle();
      break;
    case LED_COMMAND_SET_TO_LEVEL:
      zr_set_to_level(ptr);
      break;
    case LED_COMMAND_STEP_UP:
      zr_step_up();
      break;
    case LED_COMMAND_STEP_DOWN:
      zr_step_down();
      break;
    }
}

void zr_on()
{ 
  led_state = ZB_TRUE;
  TRACE_MSG(TRACE_APS2, "ON, %d", (FMT__D, led_state));
}
  
void zr_off()
{
  led_state = ZB_FALSE;
  TRACE_MSG(TRACE_APS2, "OFF, %d", (FMT__D, led_state));
}

void zr_toggle()
{
  led_state = !led_state;
  TRACE_MSG(TRACE_APS2, "TOGGLE, %d", (FMT__D, led_state));
}

void zr_set_to_level(zb_uint8_t *ptr)
{
  led_brightness += ptr[SECOND_BYTE];
  TRACE_MSG(TRACE_APS2, "SET_TO_LEVEL, %d", (FMT__D, led_brightness));
}

void zr_step_up()
{
  led_brightness += LED_BRIGHTNESS_STEP;
  TRACE_MSG(TRACE_APS2, "STEP_UP, %d", (FMT__D, led_brightness));
}

void zr_step_down()
{
  led_brightness -= LED_BRIGHTNESS_STEP;
  TRACE_MSG(TRACE_APS2, "STEP_DOWN, %d", (FMT__D, led_brightness));
}
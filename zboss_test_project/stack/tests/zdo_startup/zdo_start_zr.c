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

static void zr_send_data(zb_uint8_t param);
static void zr_profile(zb_uint8_t param);

zb_ieee_addr_t g_zr_addr = {0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb};

static volatile zb_uint8_t button_left;
static volatile zb_uint8_t button_right;
static zb_uint8_t read_pin, cycle;

void EXTI0_IRQHandler(void)
{
  EXTI_ClearITPendingBit(EXTI_Line0);
  read_pin = GPIO_ReadInputDataBit(GPIOE, B_LEFT);
  
  while(read_pin == 1)
    {
      read_pin = GPIO_ReadInputDataBit(GPIOE, B_LEFT);
      
      if(cycle == 60)
	{
	  button_left = 1;
	  break;
	}
      else
	{
	  button_left = 0;
   	}
      cycle++;
    }
  cycle = 0;
}

void EXTI1_IRQHandler(void)
{
  EXTI_ClearITPendingBit(EXTI_Line1);
  read_pin = GPIO_ReadInputDataBit(GPIOE, B_RIGHT);
  
  while(read_pin == 1)
    {
      read_pin = GPIO_ReadInputDataBit(GPIOE, B_RIGHT);
      
      if(cycle == 60)
	{
	  button_right = 1;
	  break;
	}
      else
	{
	  button_right = 0;
	}
      cycle++;
    }
  cycle = 0;
}

MAIN()
{
  
  ARGV_UNUSED;

#if !(defined KEIL || defined SDCC|| defined ZB_IAR)
  if ( argc < 3 )
  {
    return 0;
  }
#endif

  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_zr", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zr", "2", "2");
#endif
#ifdef ZB_SECURITY
  ZG->nwk.nib.security_level = 0;
#endif

  {
    zb_address_ieee_ref_t ref;
    zb_address_update(g_zr_addr, 0, ZB_FALSE, &ref); // legacy
    ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_zr_addr);
    ZB_AIB().aps_channel_mask = (1l << 22);
    init_button();
  }

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

  if (buf->u.hdr.status == 0)
    {
      ZB_SCHEDULE_CALLBACK(zr_profile, param);
    }
  else
    {
      zb_free_buf(buf);
    }
}

static void zr_send_data(zb_uint8_t param)
{
  zb_buf_t *buf = (zb_buf_t*)ZB_BUF_FROM_REF(param);
  zb_apsde_data_req_t *req;
    
  req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
  req->dst_addr.addr_short = 0;
  req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
  req->radius = 1;
  req->profileid = 2;
  req->src_endpoint = 10;
  req->dst_endpoint = 10;
  buf->u.hdr.handle = 0x11;
  button_left = 0;
  button_right = 0;
  
  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));  
}

void zr_toggle(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  ZB_BUF_INITIAL_ALLOC(buf, 1, ptr);
  ptr[0] = (zb_uint8_t)LED_COMMAND_TOGGLE;
  zr_send_data(param);
}

void zr_step_up(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  ZB_BUF_INITIAL_ALLOC(buf, 1, ptr);
  ptr[0] = (zb_uint8_t)LED_COMMAND_STEP_UP;
  zr_send_data(param);
}

void zr_change_color(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  ZB_BUF_INITIAL_ALLOC(buf, 1, ptr);
  ptr[0] = (zb_uint8_t)LED_COMMAND_CHANGE_COLOR;
  zr_send_data(param);
}

static void zr_profile(zb_uint8_t param)
{
  
  if (button_left == 1 && button_right == 1)
    { 
      ZB_SCHEDULE_CALLBACK(zr_change_color, param);
    }
      
  if (button_left == 0 && button_right == 1)
    {
      ZB_SCHEDULE_CALLBACK(zr_toggle, param);
    }
      
  if (button_left == 1 && button_right == 0)
    {
      ZB_SCHEDULE_CALLBACK(zr_step_up, param);
    }
  
  zb_uint8_t some_param = ZB_REF_FROM_BUF(zb_get_out_buf());
  ZB_SCHEDULE_ALARM(zr_profile, some_param, 5);
}

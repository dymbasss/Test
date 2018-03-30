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
PURPOSE:
*/

#include "zdo_header_for_led.h"

#ifndef ZB_ROUTER_ROLE
#error Router role is not compiled!
#endif

#ifndef ZB_SECURITY
#error Define ZB_SECURITY
#endif

static void zc_send_data(zb_uint8_t param);

void zc_profile(zb_uint8_t param);
void zc_on(zb_uint8_t param);
void zc_off(zb_uint8_t param);
void zc_set_to_level(zb_uint8_t param);
void zc_toggle(zb_uint8_t param);
void zc_step_up(zb_uint8_t param);
void zc_step_down(zb_uint8_t param);

zb_uint8_t N = 0;

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
  ZB_INIT("zdo_zr", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zr", "2", "2");
#endif

  //ZB_PIB_RX_ON_WHEN_IDLE() = ZB_FALSE;

  /* FIXME: temporary, until neighbor table is not in nvram */
  /* add extended address of potential parent to emulate that we've already
   * been connected to it */
  {
    zb_ieee_addr_t ieee_address;
    zb_address_ieee_ref_t ref;

    ZB_64BIT_ADDR_ZERO(ieee_address);
    ieee_address[7] = 8;

    zb_address_update(ieee_address, 0, ZB_FALSE, &ref);
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
      TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
      ZB_SCHEDULE_ALARM(zc_profile, param, ZB_TIME_ONE_SECOND * 13);
    }
  else
    {
      TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
      zb_free_buf(buf);
    }
}

static void zc_send_data(zb_uint8_t param)
{
  zb_buf_t *buf = (zb_buf_t*)ZB_BUF_FROM_REF(param);
  zb_apsde_data_req_t *req;
    
  req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
  req->dst_addr.addr_short = 0; /* send to ZR */
  req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
  req->radius = 1;
  req->profileid = 2;
  req->src_endpoint = 10;
  req->dst_endpoint = 10;
  buf->u.hdr.handle = 0x11;
  
  TRACE_MSG(TRACE_APS2, "Sending apsde_data.request", (FMT__0));  
  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}

void zc_on(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  ZB_BUF_INITIAL_ALLOC(buf, 1, ptr);
  ptr[0] = (zb_int8_t)LED_COMMAND_ON;
  zc_send_data(param);
}

void zc_off(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  ZB_BUF_INITIAL_ALLOC(buf, 1, ptr);
  ptr[0] = (zb_uint8_t)LED_COMMAND_OFF;
  zc_send_data(param);
}

void zc_toggle(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  ZB_BUF_INITIAL_ALLOC(buf, 1, ptr);
  ptr[0] = (zb_uint8_t)LED_COMMAND_TOGGLE;
  zc_send_data(param);
}

typedef struct zc_set_to_level_param
{
  zb_uint8_t new_value;
} zc_set_to_level_param_new;

void zc_set_to_level(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zc_set_to_level_param_new *next = ZB_GET_BUF_PARAM(buf, zc_set_to_level_param_new);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  ZB_BUF_INITIAL_ALLOC(buf,2, ptr);
  ptr[0] = (zb_uint8_t)LED_COMMAND_SET_TO_LEVEL;
  ptr[1] = next->new_value;
  zc_send_data(param);
}

void zc_step_up(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  ZB_BUF_INITIAL_ALLOC(buf, 1, ptr);
  ptr[0] = (zb_uint8_t)LED_COMMAND_STEP_UP;
  zc_send_data(param);
}

void zc_step_down(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  ZB_BUF_INITIAL_ALLOC(buf, 1, ptr);
  ptr[0] = (zb_uint8_t)LED_COMMAND_STEP_DOWN;
  zc_send_data(param);
}

void zc_profile(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zc_set_to_level_param_new *p;

  switch(N)
    {
    case LED_COMMAND_ON:
      ZB_SCHEDULE_CALLBACK(zc_on, param);
      break;
    case LED_COMMAND_OFF:
      ZB_SCHEDULE_CALLBACK(zc_off, param);
      break;
    case LED_COMMAND_TOGGLE:
      ZB_SCHEDULE_CALLBACK(zc_toggle, param);
      break;
    case LED_COMMAND_SET_TO_LEVEL:
      p = ZB_GET_BUF_PARAM(buf, zc_set_to_level_param_new); 
      p->new_value = 10;
      ZB_SCHEDULE_CALLBACK(zc_set_to_level, param);
      break;
    case LED_COMMAND_STEP_UP:
      ZB_SCHEDULE_CALLBACK(zc_step_up, param);
      break;
    case LED_COMMAND_STEP_DOWN:
      ZB_SCHEDULE_CALLBACK(zc_step_down, param);
      break;
    }

  N++;

  zb_uint8_t some_param = ZB_REF_FROM_BUF(zb_get_out_buf());
  ZB_SCHEDULE_ALARM(zc_profile, some_param, 3 * ZB_TIME_ONE_SECOND);
}
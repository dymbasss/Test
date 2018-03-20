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

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zb_secur.h"
#include "zb_secur_api.h"

#ifndef ZB_ROUTER_ROLE
#error Router role is not compiled!
#endif

#ifndef ZB_SECURITY
#error Define ZB_SECURITY
#endif


/*! \addtogroup ZB_TESTS */
/*! @{ */

zb_uint8_t answer[] = "Connected";
zb_uint8_t size = sizeof(answer) / sizeof(answer[0]);
zb_uint8_t answer_2[] = "Complete";
zb_uint8_t size_2 = sizeof(answer_2) / sizeof(answer_2[0]);

static void send_data(zb_buf_t *buf, zb_uint8_t packet[], zb_uint8_t size);
void tar(zb_uint8_t *ptr);

#ifndef APS_RETRANSMIT_TEST
void data_indication(zb_uint8_t param) ZB_CALLBACK;
#endif

/*
  ZR joins to ZC, then sends APS packet.
 */


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


void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  if (buf->u.hdr.status == 0)
    {
      TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
#ifndef APS_RETRANSMIT_TEST
      zb_af_set_data_indication(data_indication);
#endif
      send_data((zb_buf_t *)ZB_BUF_FROM_REF(param), answer, size);
    }
  else
    {
      TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
      zb_free_buf(buf);
    }
}


static void send_data(zb_buf_t *buf, zb_uint8_t packet[], zb_uint8_t size)
{
  zb_apsde_data_req_t *req;
  zb_uint8_t *ptr = NULL;
  zb_short_t i;

  ZB_BUF_INITIAL_ALLOC(buf, size, ptr);
  req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
  req->dst_addr.addr_short = 0; /* send to ZC */
  req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
  req->radius = 1;
  req->profileid = 2;
  req->src_endpoint = 10;
  req->dst_endpoint = 10;
  buf->u.hdr.handle = 0x11;

#if 0   /* test with wrong pan_id after join */
  MAC_PIB().mac_pan_id = 0x1aaa;
  ZB_UPDATE_PAN_ID();
#endif 													 
  for (i = 0 ; i < size; ++i)
    {
      ptr[i] = packet[i];
    }
  
  TRACE_MSG(TRACE_APS2, "Sending apsde_data.request", (FMT__0));
  //ZB_SCHEDULE_ALARM(zb_apsde_data_request, ZB_REF_FROM_BUF(buf), 5 * ZB_TIME_ONE_SECOND);
  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
  
}

#ifndef APS_RETRANSMIT_TEST
void data_indication(zb_uint8_t param)
{
  zb_ushort_t i;
  zb_uint8_t *ptr;
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  /* Remove APS header from the packet */
  ZB_APS_HDR_CUT_P(asdu, ptr);

  TRACE_MSG(TRACE_APS2, "data_indication: packet %p len %d handle 0x%x", (FMT__P_D_D,
                         asdu, (int)ZB_BUF_LEN(asdu), asdu->u.hdr.status));

  for (i = 0 ; i < ZB_BUF_LEN(asdu); ++i)
    {
      TRACE_MSG(TRACE_APS2, "%d %c", (FMT__D_C, ptr[i], ptr[i]));
    }

  tar(ptr);
  send_data(asdu, answer_2, size_2);
    
}
#endif

void on(zb_uint8_t *ptr, zb_ushort_t i);
void off(zb_uint8_t *ptr, zb_ushort_t i);
void toggle(zb_uint8_t *ptr, zb_ushort_t i);
void set_to_level(zb_uint8_t *ptr, zb_ushort_t i);
void step_up(zb_uint8_t *ptr, zb_ushort_t i);
void step_down(zb_uint8_t *ptr, zb_ushort_t i);

enum led_command
  {
    LED_COMMAND_ON = 1,
    LED_COMMAND_OFF,
    LED_COMMAND_TUGGLE,
    LED_COMMAND_SET_TO_LEVEL,
    LED_COMMAND_STEP_UP,
    LED_COMMAND_STEP_DOWN
  };

/* 1)on 2)off 3)toggle 4)set to level 5)step up 6)step down */

void tar(zb_uint8_t *ptr)
{
  zb_ushort_t i = 0;
  
  switch(ptr[i])
    {
    case LED_COMMAND_ON:
      on(ptr,1);
      break;
    case LED_COMMAND_OFF:
      off(ptr,1);
      break;
    case LED_COMMAND_TUGGLE:
      toggle(ptr,1);
      break;
    case LED_COMMAND_SET_TO_LEVEL:
      set_to_level(ptr,1);
      break;
    case LED_COMMAND_STEP_UP:
      step_up(ptr,1);
      break;
    case LED_COMMAND_STEP_DOWN:
      step_down(ptr,1);
      break;
    }
}

void on(zb_uint8_t *ptr, zb_ushort_t i)
{ 
  if ((ptr[i]) == 1)
    {
      TRACE_MSG(TRACE_APS2, "Status.LED -> ON", (FMT__0));
    }
}
  
void off(zb_uint8_t *ptr, zb_ushort_t i)
{
  if ((ptr[i]) == 1)
    {
      TRACE_MSG(TRACE_APS2, "Status.LED -> OFF", (FMT__0));
    }
}

void toggle(zb_uint8_t *ptr, zb_ushort_t i)
{
  switch(ptr[i])
    {
    case 1:
      TRACE_MSG(TRACE_APS2, "Steps.LED -> GREEN",(FMT__0));
      break;
    case 2:
      TRACE_MSG(TRACE_APS2, "Steps.LED -> RED",(FMT__0));
      break;
    case 3:
      TRACE_MSG(TRACE_APS2, "Steps.LED -> YELLOW",(FMT__0));
      break;
    case 4:
      TRACE_MSG(TRACE_APS2, "Steps.LED -> BLUE",(FMT__0));
      break;
    }
}

void step_up(zb_uint8_t *ptr, zb_ushort_t i)
{
  TRACE_MSG(TRACE_APS2, "Step_up.LED -> %d",(FMT__D,ptr[i]));
}

void step_down(zb_uint8_t *ptr, zb_ushort_t i)
{
  TRACE_MSG(TRACE_APS2, "Step_down.LED -> %d",(FMT__D,ptr[i]));
}

void set_to_level(zb_uint8_t *ptr, zb_ushort_t i)
{
  TRACE_MSG(TRACE_APS2, "Set_to_level.LED -> %d",(FMT__D,ptr[i]));
}
/*! @} */

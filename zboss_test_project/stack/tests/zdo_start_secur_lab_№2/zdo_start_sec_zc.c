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

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zb_secur.h"
#include "zb_secur_api.h"


/*! \addtogroup ZB_TESTS */
/*! @{ */

#ifndef ZB_COORDINATOR_ROLE
#error Coordinator role is not compiled!
#endif

#ifndef ZB_SECURITY
#error Define ZB_SECURITY
#endif

zb_ieee_addr_t g_ieee_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x08};
zb_uint8_t g_key[16] = { 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0, 0, 0, 0, 0, 0, 0, 0};

//1)on 2)off 3)toggle 4)set to level 5)step up 6)step down

/*
  The test is: ZC starts PAN, ZR joins to it by association and send APS data packet, when ZC received packet, it sends packet to ZR, when ZR received packet, it sends packet to ZC etc.
 */


static void zc_send_data(zb_uint8_t param);
static void all_stack(zb_uint8_t param);

#define addr 0x0001

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
    zb_uint8_t some_param = ZB_REF_FROM_BUF(zb_get_out_buf());
    ZB_SCHEDULE_CALLBACK(all_stack, some_param); 
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device start FAILED status %d",
	      (FMT__D, (int)buf->u.hdr.status));
  }
  zb_free_buf(buf);
}

zb_uint8_t ran_d(zb_uint8_t i)
{
  srand((zb_uint8_t)time(NULL));

  return rand()%i + 0;
}

void on(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  ZB_BUF_INITIAL_ALLOC(buf, 2, ptr);
  ptr[0] = 0;
  ptr[1] = ran_d(1);
  zc_send_data(param);
}

void off(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  ZB_BUF_INITIAL_ALLOC(buf, 2, ptr);
  ptr[0] = 1;
  ptr[1] = ran_d(1);
  zc_send_data(param);
}

void toggle(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  ZB_BUF_INITIAL_ALLOC(buf, 2, ptr);
  ptr[0] = 2;
  ptr[1] = ran_d(3);
  zc_send_data(param);
}

void set_to_level(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  ZB_BUF_INITIAL_ALLOC(buf, 2, ptr);
  ptr[0] = 3;
  ptr[1] = ran_d(255);
  zc_send_data(param);
}

void step_up(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  ZB_BUF_INITIAL_ALLOC(buf, 2, ptr);
  ptr[0] = 4;
  ptr[1] = ran_d(255);
  zc_send_data(param);
}

void step_down(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  ZB_BUF_INITIAL_ALLOC(buf, 2, ptr);
  ptr[0] = 5;
  ptr[1] = ran_d(255);
  zc_send_data(param);
}

enum led_command
  {
    LED_COMMAND_ON = 0,
    LED_COMMAND_OFF,
    LED_COMMAND_TOGGLE,
    LED_COMMAND_SET_TO_LEVEL,
    LED_COMMAND_STEP_UP,
    LED_COMMAND_STEP_DOWN
  };

static void all_stack(zb_uint8_t param)
{  
  switch(ran_d(6))
    {
    case LED_COMMAND_ON:
      ZB_SCHEDULE_CALLBACK(on, param);
      break;
    case LED_COMMAND_OFF:
      ZB_SCHEDULE_CALLBACK(off, param);
      break;
    case LED_COMMAND_TOGGLE:
      ZB_SCHEDULE_CALLBACK(toggle, param);
      break;
    case LED_COMMAND_SET_TO_LEVEL:
      ZB_SCHEDULE_CALLBACK(set_to_level, param);
      break;
    case LED_COMMAND_STEP_UP:
      ZB_SCHEDULE_CALLBACK(step_up, param);
      break;
    case LED_COMMAND_STEP_DOWN:
      ZB_SCHEDULE_CALLBACK(step_down, param);
      break;
    }
  
  zb_uint8_t some_param = ZB_REF_FROM_BUF(zb_get_out_buf());
  ZB_SCHEDULE_ALARM(all_stack, some_param,5 * ZB_TIME_ONE_SECOND); 
}


static void zc_send_data(zb_uint8_t param)
{
  zb_buf_t *buf = (zb_buf_t*)ZB_BUF_FROM_REF(param);
  zb_apsde_data_req_t *req;
    
  req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
  req->dst_addr.addr_short = addr; /* send to ZR */
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

/*! @} */

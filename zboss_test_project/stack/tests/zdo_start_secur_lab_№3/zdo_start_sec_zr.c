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



//void zr_request_information(zb_uint8_t param);
void send_data_req(zb_uint8_t param);
//void data_indication(zb_uint8_t param);

// IEEE_ADDRESS 
void IEEE_addr_req(zb_uint8_t param);
void ieee_addr_callback(zb_uint8_t param);
// NODE_DESCRIPTOR
void NODE_desc_req(zb_uint8_t param);
void node_desc_callback(zb_uint8_t param);
// POWER_DESCRIPTOR
void POWER_desc_req(zb_uint8_t param);
void power_desc_callback(zb_uint8_t param);
// SIMPLE_DESCRIPTOR
void SIMPLE_desc_req(zb_uint8_t param);
void simple_desc_callback(zb_uint8_t param);
// ACTIVE_EP_DESCRIPTOR
void ACTIVE_ep_desc_req(zb_uint8_t param);
void active_ep_callback(zb_uint8_t param);

zb_uint8_t N = 0;
zb_uint8_t end_point;

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
    ZB_SCHEDULE_CALLBACK(IEEE_addr_req, param);
    //zb_af_set_data_indication(data_indication);
    //ZB_SCHEDULE_ALARM(zr_request_information, param, ZB_TIME_ONE_SECOND);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}

typedef struct end_poit
{
  zb_uint8_t number_end_p;
} end_p;

void IEEE_addr_req(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_ieee_addr_req_t *req;
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_ieee_addr_req_t), req);

  req->nwk_addr = 0; //send to coordinator
  req->request_type = ZB_ZDO_SINGLE_DEV_RESPONSE;
  req->start_index = 0;
  zb_zdo_ieee_addr_req(ZB_REF_FROM_BUF(buf), ieee_addr_callback);
}

void ieee_addr_callback(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_nwk_addr_resp_head_t *resp;
  zb_ieee_addr_t ieee_addr;
  zb_uint16_t nwk_addr;
  zb_address_ieee_ref_t addr_ref;

  TRACE_MSG(TRACE_ZDO2, "zb_get_peer_short_addr_cb param %hd", (FMT__H, param));

  resp = (zb_zdo_nwk_addr_resp_head_t*)ZB_BUF_BEGIN(buf);
  TRACE_MSG(TRACE_ZDO2, "resp status %hd, nwk addr %d", (FMT__H_D, resp->status, resp->nwk_addr));
  ZB_DUMP_IEEE_ADDR(resp->ieee_addr);
  if (resp->status == ZB_ZDP_STATUS_SUCCESS)
  {
    ZB_LETOH64(ieee_addr, resp->ieee_addr);
    ZB_LETOH16(&nwk_addr, &resp->nwk_addr);
    zb_address_update(ieee_addr, nwk_addr, ZB_TRUE, &addr_ref);
  }
  POWER_desc_req(param);
  //zb_free_buf(buf);
}

/*void NODE_desc_req(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_node_desc_req_t *req = NULL;
  ZB_BUF_INITIAL_ALLOC(asdu, sizeof(zb_zdo_node_desc_req_t), req);

  req->nwk_addr = 0; //send to coordinator
  zb_zdo_node_desc_req(ZB_REF_FROM_BUF(asdu), node_desc_callback);
}

void node_desc_callback(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_node_desc_resp_t *resp = (zb_zdo_node_desc_resp_t*)(zdp_cmd);

  TRACE_MSG(TRACE_APS1, "node_desc_callback: status %hd, addr 0x%x",
            (FMT__H_D, resp->hdr.status, resp->hdr.nwk_addr));
  if (resp->hdr.status != ZB_ZDP_STATUS_SUCCESS || resp->hdr.nwk_addr != 0x0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect status/addr", (FMT__0));
  }

  TRACE_MSG(TRACE_APS1, "logic type %hd, aps flag %hd, frequency %hd",
            (FMT__H_H_H, ZB_GET_NODE_DESC_LOGICAL_TYPE(&resp->node_desc), ZB_GET_NODE_DESC_APS_FLAGS(&resp->node_desc),
             ZB_GET_NODE_DESC_FREQ_BAND(&resp->node_desc)));
  if (ZB_GET_NODE_DESC_LOGICAL_TYPE(&resp->node_desc) != 0 || ZB_GET_NODE_DESC_APS_FLAGS(&resp->node_desc) != 0 ||
      ZB_GET_NODE_DESC_FREQ_BAND(&resp->node_desc) != ZB_FREQ_BAND_2400 )
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect type/flags/freq", (FMT__0));
  }

  TRACE_MSG(TRACE_APS1, "mac cap 0x%hx, manufact code %hd, max buf %hd, max transfer %hd",
            (FMT__H_H_H_H, resp->node_desc.mac_capability_flags, resp->node_desc.manufacturer_code,
             resp->node_desc.max_buf_size, resp->node_desc.max_incoming_transfer_size));
  if ((resp->node_desc.mac_capability_flags & 0xB) != 0xB || (resp->node_desc.mac_capability_flags & ~0x4f) != 0 ||
      resp->node_desc.manufacturer_code != 0 ||
      resp->node_desc.max_incoming_transfer_size != 0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect cap/manuf code/max transfer", (FMT__0));
  }

  zb_free_buf(buf);
}*/

void POWER_desc_req(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_power_desc_req_t *req;
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_power_desc_req_t), req);

  req->nwk_addr = 0; //send to coordinator
  zb_zdo_power_desc_req(ZB_REF_FROM_BUF(buf), power_desc_callback);
}

void power_desc_callback(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_power_desc_resp_t *resp = (zb_zdo_power_desc_resp_t*)(zdp_cmd);


  TRACE_MSG(TRACE_APS1, " node_power_desc_callback status %hd, addr 0x%x",
            (FMT__H, resp->hdr.status, resp->hdr.nwk_addr));
  if (resp->hdr.status != ZB_ZDP_STATUS_SUCCESS || resp->hdr.nwk_addr != 0x0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect status/addr", (FMT__0));
  }

  TRACE_MSG(TRACE_APS1, "power mode %hd, avail power src %hd, cur power src %hd, cur power level %hd",
            (FMT__H_H_H_H, ZB_GET_POWER_DESC_CUR_POWER_MODE(&resp->power_desc),
             ZB_GET_POWER_DESC_AVAIL_POWER_SOURCES(&resp->power_desc),
             ZB_GET_POWER_DESC_CUR_POWER_SOURCE(&resp->power_desc),
             ZB_GET_POWER_DESC_CUR_POWER_SOURCE_LEVEL(&resp->power_desc)));

  if (ZB_GET_POWER_DESC_CUR_POWER_MODE(&resp->power_desc) != 0 ||
      ZB_GET_POWER_DESC_AVAIL_POWER_SOURCES(&resp->power_desc) != 0x7 ||
      ZB_GET_POWER_DESC_CUR_POWER_SOURCE(&resp->power_desc) != 0x1 ||
      ZB_GET_POWER_DESC_CUR_POWER_SOURCE_LEVEL(&resp->power_desc) != 0xC)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect power desc", (FMT__0));
  }
  ACTIVE_ep_desc_req(param);
  //zb_free_buf(buf);
}

void SIMPLE_desc_req(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_simple_desc_req_t *req;
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_simple_desc_req_t), req);
  req->nwk_addr = 0; //send to coordinator
  req->endpoint = end_point;
  zb_zdo_simple_desc_req(ZB_REF_FROM_BUF(buf), simple_desc_callback);
}

void simple_desc_callback(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_simple_desc_resp_t *resp = (zb_zdo_simple_desc_resp_t*)(zdp_cmd);
  zb_uint_t i;

  TRACE_MSG(TRACE_APS1, "simple_desc_callback status %hd, addr 0x%x",
            (FMT__H, resp->hdr.status, resp->hdr.nwk_addr));
  if (resp->hdr.status != ZB_ZDP_STATUS_SUCCESS || resp->hdr.nwk_addr != 0x0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect status/addr", (FMT__0));
  }
  
  TRACE_MSG(TRACE_APS1, "ep %hd, app prof %d, dev id %d, dev ver %hd, input count 0x%hx, output count 0x%hx",
            (FMT__H_D_D_H_H_H, resp->simple_desc.endpoint, resp->simple_desc.app_profile_id,
            resp->simple_desc.app_device_id, resp->simple_desc.app_device_version,
           resp->simple_desc.app_input_cluster_count, resp->simple_desc.app_output_cluster_count));

  TRACE_MSG(TRACE_APS1, "clusters:", (FMT__0));
  for(i = 0; i < resp->simple_desc.app_input_cluster_count + resp->simple_desc.app_output_cluster_count; i++)
  {
    TRACE_MSG(TRACE_APS1, " 0x%hx", (FMT__H, *(resp->simple_desc.app_cluster_list + i)));
  }
  send_data_req(param);
  //zb_free_buf(buf);
}

void ACTIVE_ep_desc_req(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_active_ep_req_t *req;
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_active_ep_req_t), req);

  req->nwk_addr = 0; //send to coordinator
  zb_zdo_active_ep_req(ZB_REF_FROM_BUF(buf), active_ep_callback);
}

void active_ep_callback(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_ep_resp_t *resp = (zb_zdo_ep_resp_t*)zdp_cmd;
  zb_uint8_t *ep_list = zdp_cmd + sizeof(zb_zdo_ep_resp_t);

  TRACE_MSG(TRACE_APS1, "active_ep_callback status %hd, addr 0x%x",
            (FMT__H, resp->status, resp->nwk_addr));

  if (resp->status != ZB_ZDP_STATUS_SUCCESS || resp->nwk_addr != 0x0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect status/addr", (FMT__0));
  }

  TRACE_MSG(TRACE_APS1, " ep count %hd, ep %hd", (FMT__H_H, resp->ep_count, *ep_list));
  end_point = *ep_list;
  if (resp->ep_count != 1 || *ep_list != 1)
  {
    TRACE_MSG(TRACE_APS3, "Error incorrect ep count or ep value", (FMT__0));
  }
  SIMPLE_desc_req(param);
  //zb_free_buf(buf);
}

void send_data_req(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsde_data_req_t *req;
  zb_uint8_t *ptr = NULL;
  
  ZB_BUF_INITIAL_ALLOC(buf, 5, ptr);
  req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
  req->dst_addr.addr_short = 0;
  req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;

  req->radius = 1;
  req->profileid = 2;
  req->clusterid = 43690;
  req->src_endpoint = 11;
  req->dst_endpoint = 1;
  buf->u.hdr.handle = 0x11;

    ptr[0] = 4; ptr[1] = 3; ptr[2] = 6; ptr[3] = 9; ptr[4] = 2;

  TRACE_MSG(TRACE_APS2, "Sending cluster.request", (FMT__0));  
  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}

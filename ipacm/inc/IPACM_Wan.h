/*
Copyright (c) 2013-2021, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Changes from Qualcomm Innovation Center are provided under the following license:

Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
     with the distribution.

   * Neither the name of Qualcomm Innovation Center, Inc. nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
*/
/*
 * ​​​​​Changes from Qualcomm Innovation Center are provided under the following license:
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
/*!
	@file
	IPACM_Wan.cpp

	@brief
	This file implements the WAN iface functionality.

	@Author
	Skylar Chang

*/
#ifndef IPACM_WAN_H
#define IPACM_WAN_H

#include <stdio.h>
#include <IPACM_CmdQueue.h>
#include <linux/msm_ipa.h>
#include "IPACM_Routing.h"
#include "IPACM_Filtering.h"
#include <IPACM_Iface.h>
#include <IPACM_Defs.h>
#include <IPACM_Xml.h>

#define IPA_NUM_DEFAULT_WAN_FILTER_RULES 3 /*1 for v4, 2 for v6*/
#define IPA_V2_NUM_DEFAULT_WAN_FILTER_RULE_IPV4 2

#ifdef FEATURE_IPA_ANDROID
#define IPA_V2_NUM_DEFAULT_WAN_FILTER_RULE_IPV6 4
#define IPA_V2_NUM_TCP_WAN_FILTER_RULE_IPV6 3
#define IPA_V2_NUM_MULTICAST_WAN_FILTER_RULE_IPV6 3
#define IPA_V2_NUM_FRAG_WAN_FILTER_RULE_IPV6 1
#else
#define IPA_V2_NUM_DEFAULT_WAN_FILTER_RULE_IPV6 3
#endif
#define MAX_DEFAULT_SEC_v6_ROUTE_RULES  1

#define NETWORK_STATS "%s %llu %llu %llu %llu"
#define IPA_NETWORK_STATS_FILE_NAME "/data/misc/ipa/network_stats"
#define IPA_OFFLOAD_TETHER_STATE_FILE_NAME "/data/vendor/ipa/offload_state"

typedef struct _wan_client_rt_hdl
{
	uint32_t wan_rt_rule_hdl_v4;
	uint32_t wan_rt_rule_hdl_v6[IPV6_NUM_ADDR];
	uint32_t wan_rt_rule_hdl_v6_wan[IPV6_NUM_ADDR];
}wan_client_rt_hdl;

typedef struct _ipa_wan_client
{
	ipacm_event_data_wlan_ex* p_hdr_info;
	uint8_t mac[IPA_MAC_ADDR_SIZE];
	uint32_t v4_addr;
	uint32_t v6_addr[IPV6_NUM_ADDR][4];
	uint32_t hdr_hdl_v4;
	uint32_t hdr_hdl_v6;
	bool route_rule_set_v4;
	int route_rule_set_v6;
	bool ipv4_set;
	int ipv6_set;
	bool ipv4_header_set;
	bool ipv6_header_set;
	bool power_save_set;
	wan_client_rt_hdl wan_rt_hdl[0]; /* depends on number of tx properties */
}ipa_wan_client;

typedef struct
{
	bool coalesce_tcp_enable;
	bool coalesce_udp_enable;
}ipacm_coalesce;

/* wan iface */
class IPACM_Wan : public IPACM_Iface
{

public:
	/* IPACM pm_depency q6 check*/
	static int ipa_pm_q6_check;
	static bool wan_up;
	static bool wan_up_v6;
	static uint8_t xlat_mux_id;

	static uint16_t mtu_default_wan_v4;
	static uint16_t mtu_default_wan_v6;

	/* IPACM interface name */
	static char wan_up_dev_name[IF_NAME_LEN];
	static uint32_t curr_wan_ip;
	IPACM_Wan(int, ipacm_wan_iface_type, uint8_t *);
	virtual ~IPACM_Wan();

	static bool isWanUP(int ipa_if_num_tether)
	{
#ifdef FEATURE_IPA_ANDROID
#ifdef FEATURE_IPACM_AIDL
		/*To avoid -Wall -Werror error */
		IPACMDBG_H("ipa_if_num_tether: %d\n",ipa_if_num_tether);
		return wan_up;
#else
		uint32_t i;
		for (i=0; i < ipa_if_num_tether_v4_total;i++)
		{
			if (ipa_if_num_tether_v4[i] == ipa_if_num_tether)
			{
				IPACMDBG_H("support ipv4 tether_iface(%s)\n",
					IPACM_Iface::ipacmcfg->iface_table[ipa_if_num_tether].iface_name);
				return wan_up;
				break;
			}
		}
		return false;
#endif
#else
		return wan_up;
#endif
	}

	static uint16_t queryMTU(int ipa_if_num_tether, enum ipa_ip_type iptype)
	{
		if (iptype == IPA_IP_v4)
		{
			if (isWanUP(ipa_if_num_tether))
			{
				return mtu_default_wan_v4;
			}
		}
		else if (iptype == IPA_IP_v6)
		{
			if (isWanUP_V6(ipa_if_num_tether))
			{
				return mtu_default_wan_v6;
			}
		}
		return DEFAULT_MTU_SIZE;
	}

	static bool isWanUP_V6(int ipa_if_num_tether)
	{
#ifdef FEATURE_IPA_ANDROID
#ifdef FEATURE_IPACM_AIDL
		/*To avoid -Wall -Werror error */
		IPACMDBG_H("ipa_if_num_tether: %d\n",ipa_if_num_tether);
		return wan_up_v6;
#else
		uint32_t i;
		for (i=0; i < ipa_if_num_tether_v6_total;i++)
		{
			if (ipa_if_num_tether_v6[i] == ipa_if_num_tether)
			{
				IPACMDBG_H("support ipv6 tether_iface(%s)\n",
					IPACM_Iface::ipacmcfg->iface_table[ipa_if_num_tether].iface_name);
				return wan_up_v6;
				break;
			}
		}
		return false;
#endif
#else
		return wan_up_v6;
#endif
	}

#ifdef FEATURE_IPA_ANDROID
	static int delete_tether_iface(ipa_ip_type iptype, int ipa_if_num_tether)
	{
		uint32_t i, j;

		if (iptype == IPA_IP_v4)
		{
			/* delete support tether ifaces to its array*/
			for (i=0; i < IPACM_Wan::ipa_if_num_tether_v4_total; i++)
			{
				if(IPACM_Wan::ipa_if_num_tether_v4[i] == ipa_if_num_tether)
				{
					IPACMDBG_H("Found tether client at position %d name(%s)\n", i,
					IPACM_Iface::ipacmcfg->iface_table[ipa_if_num_tether].iface_name);
					break;
				}
			}
			if(i == IPACM_Wan::ipa_if_num_tether_v4_total)
			{
				IPACMDBG_H("Not finding the tethered ipv4 client.\n");
				return IPACM_FAILURE;
			}
			for(j = i+1; j < IPACM_Wan::ipa_if_num_tether_v4_total; j++)
			{
				IPACM_Wan::ipa_if_num_tether_v4[j-1] = IPACM_Wan::ipa_if_num_tether_v4[j];
			}
			IPACM_Wan::ipa_if_num_tether_v4_total--;
			IPACMDBG_H("Now the total num of ipa_if_num_tether_v4_total is %d\n",
				IPACM_Wan::ipa_if_num_tether_v4_total);
		}
		else
		{
			/* delete support tether ifaces to its array*/
			for (i=0; i < IPACM_Wan::ipa_if_num_tether_v6_total; i++)
			{
				if(IPACM_Wan::ipa_if_num_tether_v6[i] == ipa_if_num_tether)
				{
					IPACMDBG_H("Found tether client at position %d name(%s)\n", i,
					IPACM_Iface::ipacmcfg->iface_table[ipa_if_num_tether].iface_name);
					break;
				}
			}
			if(i == IPACM_Wan::ipa_if_num_tether_v6_total)
			{
				IPACMDBG_H("Not finding the tethered ipv6 client.\n");
				return IPACM_FAILURE;
			}
			for(j = i+1; j < IPACM_Wan::ipa_if_num_tether_v6_total; j++)
			{
				IPACM_Wan::ipa_if_num_tether_v6[j-1] = IPACM_Wan::ipa_if_num_tether_v6[j];
			}
			IPACM_Wan::ipa_if_num_tether_v6_total--;
			IPACMDBG_H("Now the total num of ipa_if_num_tether_v6_total is %d\n",
				IPACM_Wan::ipa_if_num_tether_v6_total);
		}
		return IPACM_SUCCESS;
	}
#endif
	static void coalesce_config(uint8_t qmap_id, bool tcp_enable, bool udp_enable)
	{
		if (qmap_id >= IPA_MAX_NUM_SW_PDNS)
		{
			IPACMERR("qmap_id (%d) beyond the Max range (%d), abort\n",
				qmap_id, IPA_MAX_NUM_SW_PDNS);
			return ;
		}

		IPACM_Wan::coalesce_enable_info[qmap_id].coalesce_tcp_enable = tcp_enable;
		IPACM_Wan::coalesce_enable_info[qmap_id].coalesce_udp_enable = udp_enable;
		IPACMDBG_H(" Updated qmap(%d) coalesce enable TCP:%d UDP:%d\n",
				qmap_id,
				IPACM_Wan::coalesce_enable_info[qmap_id].coalesce_tcp_enable,
				IPACM_Wan::coalesce_enable_info[qmap_id].coalesce_udp_enable);
		return ;
	}

	static void coalesce_config_reset()
	{
		int i;
		/* reset coalesce settings on all modem interfaces */
		for (i = 0; i < IPA_MAX_NUM_SW_PDNS; i++)
			IPACM_Wan::coalesce_config(i, false, false);
		return ;
	}

	static uint32_t getWANIP()
	{
		return curr_wan_ip;
	}

	static int getXlat_Mux_Id()
	{
		if (is_xlat)
		{
			IPACMDBG_H("xlat_mux_id: %d\n", xlat_mux_id);
			return xlat_mux_id;
		} else {
			IPACMDBG_H("no xlat return invalid mux-id: 0\n");
			return 0;
		}
	}

	static void clearExtProp()
	{
		IPACM_Wan::is_ext_prop_set = false;
		IPACM_Iface::ipacmcfg->DelExtProp(IPA_IP_v4);
		IPACM_Iface::ipacmcfg->DelExtProp(IPA_IP_v6);
	}

	void event_callback(ipa_cm_event_id event,
											void *data);

	static struct ipa_flt_rule_add flt_rule_v4[IPA_MAX_FLT_RULE];
	static struct ipa_flt_rule_add flt_rule_v6[IPA_MAX_FLT_RULE];

	static int num_v4_flt_rule;
	static int num_v6_flt_rule;

	ipacm_wan_iface_type m_is_sta_mode;
	static ipacm_wan_iface_type backhaul_mode;
	static bool is_ext_prop_set;
	static uint32_t backhaul_ipv6_prefix[2];

	static bool embms_is_on;
	static bool backhaul_is_wan_bridge;
	static bool is_xlat;

	static bool isWan_Bridge_Mode()
	{
		return backhaul_is_wan_bridge;
	}
#ifdef FEATURE_IPA_ANDROID
	/* IPACM interface id */
	static uint32_t ipa_if_num_tether_v4_total;
	static int ipa_if_num_tether_v4[IPA_MAX_IFACE_ENTRIES];
	static uint32_t ipa_if_num_tether_v6_total;
	static int ipa_if_num_tether_v6[IPA_MAX_IFACE_ENTRIES];

	static bool isXlat()
	{
		return is_xlat;
	}
#endif

	/* indicate coalesce support on tcp or udp*/
	static ipacm_coalesce coalesce_enable_info[IPA_MAX_NUM_SW_PDNS];

private:

	bool is_ipv6_frag_firewall_flt_rule_installed;
	uint32_t ipv6_frag_firewall_flt_rule_hdl;
	uint32_t *wan_route_rule_v4_hdl;
	uint32_t *wan_route_rule_v6_hdl;
	uint32_t *wan_route_rule_v6_hdl_a5;
	uint32_t hdr_hdl_sta_v4;
	uint32_t hdr_hdl_sta_v6;
	uint32_t firewall_hdl_v4[IPACM_MAX_FIREWALL_ENTRIES];
	uint32_t firewall_hdl_v6[IPACM_MAX_FIREWALL_ENTRIES];
	uint32_t dft_wan_fl_hdl[IPA_NUM_DEFAULT_WAN_FILTER_RULES];
	uint32_t ipv6_dest_flt_rule_hdl[MAX_DEFAULT_v6_ROUTE_RULES];
	int num_ipv6_dest_flt_rule;
	uint32_t ODU_fl_hdl[IPA_NUM_DEFAULT_WAN_FILTER_RULES];
	int num_firewall_v4,num_firewall_v6;
	uint32_t wan_v4_addr;
	uint32_t sec_wan_v4_addr;
	bool sec_wan_v4_addr_set;
	uint32_t wan_v4_addr_gw;
	uint32_t wan_v6_addr_gw[4];
	bool wan_v4_addr_set;
	bool wan_v4_addr_gw_set;
	bool wan_v6_addr_gw_set;
	bool active_v4;
	bool active_v6;
	bool header_set_v4;
	bool header_set_v6;
	bool header_partial_default_wan_v4;
	bool header_partial_default_wan_v6;
	uint8_t ext_router_mac_addr[IPA_MAC_ADDR_SIZE];
	uint8_t netdev_mac[IPA_MAC_ADDR_SIZE];
	/* IPACM interface secondary v6 ip-addresses */
	uint32_t sec_ipv6_addr[MAX_DEFAULT_SEC_v6_ROUTE_RULES][4];
	/* create additional set of v4 Coalesce RT-rules: tcp udp */
	uint32_t dft_coalesce_rt_rule_hdl[2*MAX_DEFAULT_v4_ROUTE_RULES+ 2*MAX_DEFAULT_v6_ROUTE_RULES];
	/* create additional set of v4 Coalesce RT-rules for secondary addresses: tcp udp */
	uint32_t sec_dft_coalesce_rt_rule_hdl[2*MAX_DEFAULT_v4_ROUTE_RULES+ 2*MAX_DEFAULT_SEC_v6_ROUTE_RULES];

	/* create additional set of v4/v6 low_lat RT-rules: tcp udp */
	uint32_t dft_low_lat_rt_rule_hdl[MAX_DEFAULT_v4_ROUTE_RULES+ MAX_DEFAULT_v6_ROUTE_RULES];

	/* create additional set of v4/v6 low_lat RT-rules for seconday addresses: */
	uint32_t sec_dft_low_lat_rt_rule_hdl[MAX_DEFAULT_v4_ROUTE_RULES+ MAX_DEFAULT_SEC_v6_ROUTE_RULES];

	static int num_ipv4_modem_pdn;

	static int num_ipv6_modem_pdn;

	int modem_ipv4_pdn_index;

	int modem_ipv6_pdn_index;

	bool is_default_gateway;

	uint32_t ipv6_prefix[2];

	/* IPACM firewall Configuration file*/
	IPACM_firewall_conf_t firewall_config;

	/* STA mode wan-client*/
	int wan_client_len;
	ipa_wan_client *wan_client;
	int header_name_count;
	uint32_t num_wan_client;
	uint8_t invalid_mac[IPA_MAC_ADDR_SIZE];
	bool is_xlat_local;

	/* update network stats for CNE */
	int ipa_network_stats_fd;
	uint32_t hdr_hdl_dummy_v6;
	uint32_t hdr_proc_hdl_dummy_v6;

	/* handle for UDP mhi frag rule */
	uint32_t mhi_dl_v4_frag_hdl;

	/* handle for icmpv6 exception rule */
	uint32_t icmpv6_exception_hdl;

	/* handle for TCP FIN rule */
	uint32_t tcp_fin_hdl;

	/* handle for TCP RST rule */
	uint32_t tcp_rst_hdl;

	/* V4 MTU value. */
	uint16_t mtu_v4;
	bool mtu_v4_set;

	/* V6 MTU value. */
	uint16_t mtu_v6;
	bool mtu_v6_set;

	/* IPACM number of default route rules for secondary ipv6 address*/
	uint32_t sec_num_dft_rt_v6;

	/* create additional set of v6 RT-rules for secondary addresses in Wanv6RT table*/
	uint32_t sec_dft_rt_rule_hdl[MAX_DEFAULT_v4_ROUTE_RULES + 2*MAX_DEFAULT_SEC_v6_ROUTE_RULES];

	inline ipa_wan_client* get_client_memptr(ipa_wan_client *param, int cnt)
	{
	    char *ret = ((char *)param) + (wan_client_len * cnt);
		return (ipa_wan_client *)ret;
	}

	inline int get_wan_client_index(uint8_t *mac_addr)
	{
		int cnt;
		int num_wan_client_tmp = num_wan_client;

		IPACMDBG_H("Passed MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
						 mac_addr[0], mac_addr[1], mac_addr[2],
						 mac_addr[3], mac_addr[4], mac_addr[5]);

		for(cnt = 0; cnt < num_wan_client_tmp; cnt++)
		{
			IPACMDBG_H("stored MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
							 get_client_memptr(wan_client, cnt)->mac[0],
							 get_client_memptr(wan_client, cnt)->mac[1],
							 get_client_memptr(wan_client, cnt)->mac[2],
							 get_client_memptr(wan_client, cnt)->mac[3],
							 get_client_memptr(wan_client, cnt)->mac[4],
							 get_client_memptr(wan_client, cnt)->mac[5]);

			if(memcmp(get_client_memptr(wan_client, cnt)->mac,
								mac_addr,
								sizeof(get_client_memptr(wan_client, cnt)->mac)) == 0)
			{
				IPACMDBG_H("Matched client index: %d\n", cnt);
				return cnt;
			}
		}

		return IPACM_INVALID_INDEX;
	}

	inline int get_wan_client_index_ipv4(uint32_t ipv4_addr)
	{
		int cnt;
		int num_wan_client_tmp = num_wan_client;

		IPACMDBG_H("Passed IPv4 %x\n", ipv4_addr);

		for(cnt = 0; cnt < num_wan_client_tmp; cnt++)
		{
			if (get_client_memptr(wan_client, cnt)->ipv4_set)
			{
				IPACMDBG_H("stored IPv4 %x\n", get_client_memptr(wan_client, cnt)->v4_addr);

				if(ipv4_addr == get_client_memptr(wan_client, cnt)->v4_addr)
				{
					IPACMDBG_H("Matched client index: %d\n", cnt);
					IPACMDBG_H("The MAC is %02x:%02x:%02x:%02x:%02x:%02x\n",
							get_client_memptr(wan_client, cnt)->mac[0],
							get_client_memptr(wan_client, cnt)->mac[1],
							get_client_memptr(wan_client, cnt)->mac[2],
							get_client_memptr(wan_client, cnt)->mac[3],
							get_client_memptr(wan_client, cnt)->mac[4],
							get_client_memptr(wan_client, cnt)->mac[5]);
					IPACMDBG_H("header set ipv4(%d) ipv6(%d)\n",
							get_client_memptr(wan_client, cnt)->ipv4_header_set,
							get_client_memptr(wan_client, cnt)->ipv6_header_set);
					return cnt;
				}
			}
		}
		return IPACM_INVALID_INDEX;
	}

	inline int get_wan_client_index_ipv6(uint32_t* ipv6_addr)
	{
		int cnt, v6_num;
		int num_wan_client_tmp = num_wan_client;

		IPACMDBG_H("Get ipv6 address 0x%08x.0x%08x.0x%08x.0x%08x\n", ipv6_addr[0], ipv6_addr[1], ipv6_addr[2], ipv6_addr[3]);

		for(cnt = 0; cnt < num_wan_client_tmp; cnt++)
		{
			if (get_client_memptr(wan_client, cnt)->ipv6_set)
			{
			    for(v6_num=0;v6_num < get_client_memptr(wan_client, cnt)->ipv6_set;v6_num++)
	            {

					IPACMDBG_H("stored IPv6 0x%08x.0x%08x.0x%08x.0x%08x\n", get_client_memptr(wan_client, cnt)->v6_addr[v6_num][0],
						get_client_memptr(wan_client, cnt)->v6_addr[v6_num][1],
						get_client_memptr(wan_client, cnt)->v6_addr[v6_num][2],
						get_client_memptr(wan_client, cnt)->v6_addr[v6_num][3]);

					if(ipv6_addr[0] == get_client_memptr(wan_client, cnt)->v6_addr[v6_num][0] &&
					   ipv6_addr[1] == get_client_memptr(wan_client, cnt)->v6_addr[v6_num][1] &&
					   ipv6_addr[2]== get_client_memptr(wan_client, cnt)->v6_addr[v6_num][2] &&
					   ipv6_addr[3] == get_client_memptr(wan_client, cnt)->v6_addr[v6_num][3])
					{
						IPACMDBG_H("Matched client index: %d\n", cnt);
						IPACMDBG_H("The MAC is %02x:%02x:%02x:%02x:%02x:%02x\n",
								get_client_memptr(wan_client, cnt)->mac[0],
								get_client_memptr(wan_client, cnt)->mac[1],
								get_client_memptr(wan_client, cnt)->mac[2],
								get_client_memptr(wan_client, cnt)->mac[3],
								get_client_memptr(wan_client, cnt)->mac[4],
								get_client_memptr(wan_client, cnt)->mac[5]);
						IPACMDBG_H("header set ipv4(%d) ipv6(%d)\n",
								get_client_memptr(wan_client, cnt)->ipv4_header_set,
								get_client_memptr(wan_client, cnt)->ipv6_header_set);
						return cnt;
					}
				}
			}
		}
		return IPACM_INVALID_INDEX;
	}

	inline int delete_wan_rtrules(int clt_indx, ipa_ip_type iptype)
	{
		uint32_t tx_index;
		uint32_t rt_hdl;
		int num_v6;

		if(iptype == IPA_IP_v4)
		{
		     for(tx_index = 0; tx_index < iface_query->num_tx_props; tx_index++)
		     {
		        if((tx_prop->tx[tx_index].ip == IPA_IP_v4) && (get_client_memptr(wan_client, clt_indx)->route_rule_set_v4==true)) /* for ipv4 */
			{
				IPACMDBG_H("Delete client index %d ipv4 Qos rules for tx:%d \n",clt_indx,tx_index);
				rt_hdl = get_client_memptr(wan_client, clt_indx)->wan_rt_hdl[tx_index].wan_rt_rule_hdl_v4;

				if(m_routing.DeleteRoutingHdl(rt_hdl, IPA_IP_v4) == false)
				{
					return IPACM_FAILURE;
				}
			}
		     } /* end of for loop */

		     /* clean the 4 Qos ipv4 RT rules for client:clt_indx */
		     if(get_client_memptr(wan_client, clt_indx)->route_rule_set_v4==true) /* for ipv4 */
		     {
				get_client_memptr(wan_client, clt_indx)->route_rule_set_v4 = false;
		     }
		}

		if(iptype == IPA_IP_v6)
		{
		    for(tx_index = 0; tx_index < iface_query->num_tx_props; tx_index++)
		    {

				if((tx_prop->tx[tx_index].ip == IPA_IP_v6) && (get_client_memptr(wan_client, clt_indx)->route_rule_set_v6 != 0)) /* for ipv6 */
				{
					for(num_v6 =0;num_v6 < get_client_memptr(wan_client, clt_indx)->route_rule_set_v6;num_v6++)
					{
						IPACMDBG_H("Delete client index %d ipv6 Qos rules for %d-st ipv6 for tx:%d\n", clt_indx,num_v6,tx_index);
						rt_hdl = get_client_memptr(wan_client, clt_indx)->wan_rt_hdl[tx_index].wan_rt_rule_hdl_v6[num_v6];
						if(m_routing.DeleteRoutingHdl(rt_hdl, IPA_IP_v6) == false)
						{
							return IPACM_FAILURE;
						}

						rt_hdl = get_client_memptr(wan_client, clt_indx)->wan_rt_hdl[tx_index].wan_rt_rule_hdl_v6_wan[num_v6];
						if(m_routing.DeleteRoutingHdl(rt_hdl, IPA_IP_v6) == false)
						{
							return IPACM_FAILURE;
						}
					}

				}
			} /* end of for loop */

		    /* clean the 4 Qos ipv6 RT rules for client:clt_indx */
		    if(get_client_memptr(wan_client, clt_indx)->route_rule_set_v6 != 0) /* for ipv6 */
		    {
		                 get_client_memptr(wan_client, clt_indx)->route_rule_set_v6 = 0;
                    }
		}

		return IPACM_SUCCESS;
	}

	int handle_wan_hdr_init(uint8_t *mac_addr, bool replaced = false, int entry = 0);
	int handle_wan_client_ipaddr(ipacm_event_data_all *data);
	int handle_wan_client_route_rule(uint8_t *mac_addr, ipa_ip_type iptype);

	/* handle_gw_mac_renew, index_client valiud is success */
	int handle_gw_mac_renew(ipacm_event_data_all *data, int index_client);

	/* handle new_address event */
	int handle_addr_evt(ipacm_event_data_addr *data);

	/* handle del_address event */
	int handle_addr_del_evt(ipacm_event_data_addr *data);

	/* handle new_address event for q6_mhi */
	int handle_addr_evt_mhi_q6(ipacm_event_data_addr *data);

	/* wan default route/filter rule configuration */
	int handle_route_add_evt(ipa_ip_type iptype, bool add_only = false);

	/* construct complete STA ethernet header */
	int handle_sta_header_add_evt(bool renew = false);

	bool check_dft_firewall_rules_attr_mask(IPACM_firewall_conf_t *firewall_config);

#ifdef FEATURE_IPA_ANDROID
	/* wan posting supported tether_iface */
	int post_wan_up_tether_evt(ipa_ip_type iptype, int ipa_if_num_tether);

	int post_wan_down_tether_evt(ipa_ip_type iptype, int ipa_if_num_tether);
#endif
	int config_dft_firewall_rules(ipa_ip_type iptype);

	/* configure the initial firewall filter rules */
	int config_dft_embms_rules(ipa_ioc_add_flt_rule *pFilteringTable_v4, ipa_ioc_add_flt_rule *pFilteringTable_v6);

	int handle_route_del_evt(ipa_ip_type iptype, bool delete_only = false);

	int del_dft_firewall_rules(ipa_ip_type iptype);

	int handle_down_evt();

	/*handle wan-iface down event */
	int handle_down_evt_ex();

	/* wan default route/filter rule delete */
	int handle_route_del_evt_ex(ipa_ip_type iptype);

	/* configure the initial firewall filter rules */
	int config_dft_firewall_rules_ex(struct ipa_flt_rule_add* rules, int rule_offset,
		ipa_ip_type iptype);

	/* configure dl ack rule if enabled. */
	int config_filter_dl_ack_rule_ex(struct ipa_flt_rule_add *rules, int rule_offset,
		ipa_ip_type iptype);

	/* init filtering rule in wan dl filtering table */
	int init_fl_rule_ex(ipa_ip_type iptype);

	/* add ICMP and ALG rules in wan dl filtering table */
	int add_icmp_alg_rules(struct ipa_flt_rule_add* rules, int rule_offset, ipa_ip_type iptype);

	/* query extended property */
	int query_ext_prop();

	ipa_ioc_query_intf_ext_props *ext_prop;

	int config_wan_firewall_rule(ipa_ip_type iptype);

	int del_wan_firewall_rule(ipa_ip_type iptype);

	int add_dft_filtering_rule(struct ipa_flt_rule_add* rules, int rule_offset, ipa_ip_type iptype);

	/* add embedded destination based ipv6 rules */
	int add_embed_dstn_ipv6_rules(struct ipa_flt_rule_add* rules, int rule_offset, ipa_ip_type iptype);

	int install_wan_filtering_rule(bool is_sw_routing);

	void handle_wlan_SCC_MCC_switch(bool, ipa_ip_type);

	void handle_wan_client_SCC_MCC_switch(bool, ipa_ip_type);

	int handle_network_stats_evt();

	int m_fd_ipa;

	int handle_network_stats_update(ipa_get_apn_data_stats_resp_msg_v01 *data);

	/* construct dummy ethernet header */
	int add_dummy_rx_hdr();

	int handle_coalesce_evt();

	int add_offload_frag_rule();

	int delete_offload_frag_rule();

	int add_icmpv6_exception_rule();

	int delete_icmpv6_exception_rule();

	int add_tcp_fin_rst_exception_rule();

	int delete_tcp_fin_rst_exception_rule();

	/* MTU helper functions */
	int query_mtu_size();
};

#endif /* IPACM_WAN_H */

/* 
Copyright (c) 2013, The Linux Foundation. All rights reserved.

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
*/

#ifndef IPACM_CONNTRACK_LISTENER
#define IPACM_CONNTRACK_LISTENER

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#include "IPACM_CmdQueue.h"
#include "IPACM_Conntrack_NATApp.h"
#include "IPACM_Listener.h"

using namespace std;

class IPACM_ConntrackListener : public IPACM_Listener
{

private:
	 bool isCTReg;
	 bool isWanUp;

	 int NonNatIfaceCnt;
	 NonNatIfaces *pNonNatIfaces;
	 IPACM_Config *pConfig;
	 
	 void ProcessCTMessage(void *data);
	 void ProcessTCPorUDPMsg(struct nf_conntrack *ct, enum nf_conntrack_msg_type, u_int8_t);
	 void TriggerWANUp(void *);
	 void TriggerWANDown(uint32_t wan_addr);
	 int  CreateNatThreads(void);

	 void HandlePowerSave(void *data);
	 void HandleResetPower(void *data);
	 void HandleNeighIpAddrEvt(void *data, bool);

public:
	 char wan_ifname[IPA_IFACE_NAME_LEN];
	 uint32_t wan_ipaddr;

	 IPACM_ConntrackListener();
	 void event_callback(ipa_cm_event_id, void *data);
};

#endif /* IPACM_CONNTRACK_LISTENER */
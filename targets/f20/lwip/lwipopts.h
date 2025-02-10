/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Simon Goldschmidt
 *
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#define NO_SYS                     0
#define SYS_LIGHTWEIGHT_PROT       1
#define MEM_ALIGNMENT              4
#define LWIP_ETHERNET              1
#define LWIP_RAW                   0
#define LWIP_NETCONN               0
#define LWIP_SOCKET                1
#define LWIP_DHCP                  0
#define LWIP_ICMP                  1
#define LWIP_IGMP                  1
#define LWIP_UDP                   1
#define LWIP_TCP                   1
#define LWIP_IPV4                  1
#define LWIP_IPV6                  0
#define ETH_PAD_SIZE               0
#define LWIP_IP_ACCEPT_UDP_PORT(p) ((p) == PP_NTOHS(67))

#define TCPIP_THREAD_STACKSIZE    4096
#define TCPIP_MBOX_SIZE           6
#define DEFAULT_RAW_RECVMBOX_SIZE 6
#define DEFAULT_UDP_RECVMBOX_SIZE 6
#define DEFAULT_TCP_RECVMBOX_SIZE 6
#define DEFAULT_ACCEPTMBOX_SIZE   6
#define MEMP_NUM_SYS_TIMEOUT      17

#define TCP_MSS     (1500 /*mtu*/ - 20 /*iphdr*/ - 20 /*tcphhr*/)
#define TCP_SND_BUF (4 * TCP_MSS)
#define TCP_WND     (4 * TCP_MSS)

#define ETHARP_SUPPORT_STATIC_ENTRIES 1

#define LWIP_TIMEVAL_PRIVATE 0

#define LWIP_MDNS_RESPONDER 1

#define LWIP_HTTPD_CGI             0
#define LWIP_HTTPD_SSI             0
#define LWIP_HTTPD_SSI_INCLUDE_TAG 0
#define HTTPD_USE_CUSTOM_FSDATA    0

#define LWIP_SINGLE_NETIF 1

#define PBUF_POOL_SIZE 8 // TODO:

#define LWIP_NUM_NETIF_CLIENT_DATA 10

#define LWIP_TCPIP_CORE_LOCKING 0

#define LWIP_MULTICAST_PING           1
#define LWIP_BROADCAST_PING           1
#define LWIP_IPV6_MLD                 0
#define LWIP_IPV6_SEND_ROUTER_SOLICIT 0

#define LWIP_RAND() ((uint32_t)rand())

#endif /* __LWIPOPTS_H__ */

/*******************************************************************************
* @file  lwipopts.h
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
******************************************************************************/
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#define LWIP_NETIF_EXT_STATUS_CALLBACK 0
#define LWIP_EVENT_API                 0
#define LWIP_CALLBACK_API              1

/**
 * NO_SYS==1: Provides VERY minimal functionality. Otherwise,
 * use lwIP facilities.
 */
#define NO_SYS 0

/* ---------- NETIF API Support ---------- */
#define LWIP_NETIF_API 1

/* ---------- IPV4/IPV6 Support ---------- */
#define LWIP_IPV4 1

#ifndef LWIP_IPV6
#define LWIP_IPV6 0
#endif

/* ---------- ARP Support ---------- */
#ifndef LWIP_ARP
#define LWIP_ARP (LWIP_IPV4)
#endif

/* ---------- ICMP options ---------- */
#ifndef LWIP_ICMP
#define LWIP_ICMP (LWIP_IPV4)
#endif

/* ---------- IGMP Support ---------- */
#ifndef LWIP_IGMP
#define LWIP_IGMP (LWIP_IPV4)
#endif

/* ---------- DHCP options ---------- */
#ifndef LWIP_DHCP
#define LWIP_DHCP (LWIP_IPV4)
#endif

#if LWIP_IPV6
#define LWIP_ETHERNET                 (LWIP_IPV6)
#define LWIP_IPV6_MLD                 (LWIP_IPV6)
#define LWIP_ICMP6                    (LWIP_IPV6)
#define LWIP_IPV6_DHCP6               0
#define LWIP_IPV6_AUTOCONFIG          (LWIP_IPV6)
#define LWIP_IPV6_DUP_DETECT_ATTEMPTS 1
#define LWIP_IPV6_ND                  (LWIP_IPV6)
#endif

/* ---------- TCP options ---------- */
#define LWIP_TCP 1
#define TCP_TTL  255

/* Controls if TCP should queue segments that arrive out of order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ 0

/* TCP Maximum segment size. */
#define TCP_MSS (1500 - 40) /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF (4 * TCP_MSS)

/*  TCP_SND_QUEUELEN: TCP sender buffer space (pbufs). This must be at least as much as (2 * TCP_SND_BUF/TCP_MSS) for things to work. */
#define TCP_SND_QUEUELEN (2 * TCP_SND_BUF / TCP_MSS)

/* TCP receive window. */
#define TCP_WND (8 * TCP_MSS) //(2*TCP_MSS)

#define ETHARP_SUPPORT_STATIC_ENTRIES 1

/* ---------- UDP options ---------- */
#define LWIP_UDP 1
#define UDP_TTL  255

/* ---------- Statistics options ---------- */
#define LWIP_STATS 0

/* ---------- link callback options ---------- */
/* LWIP_NETIF_LINK_CALLBACK==1: Support a callback function from an interface whenever the link changes (i.e., link down) */
#define LWIP_NETIF_LINK_CALLBACK 1

/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: Set this value to the CPU alignment used when compiling lwIP. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT 4

/* MEM_SIZE: The size of the heap memory. If the application sends large amounts of data that must be copied, set this value higher. */
#define MEM_SIZE (3 * 1024)

/* MEMP_NUM_PBUF: The number of memp struct pbufs. If the application sends a lot of data out of ROM (or other static memory), set this value higher. */
#define MEMP_NUM_PBUF 10

/* MEMP_NUM_UDP_PCB: The number of UDP protocol control blocks. One per active UDP "connection". */
#define MEMP_NUM_UDP_PCB 6

/* MEMP_NUM_TCP_PCB: The number of simultaneously active TCP connections. */
#define MEMP_NUM_TCP_PCB 10

/* MEMP_NUM_TCP_PCB_LISTEN: The number of listening TCP connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 5

/* MEMP_NUM_TCP_SEG: The number of simultaneously queued TCP segments. */
#define MEMP_NUM_TCP_SEG 8

/* MEMP_NUM_SYS_TIMEOUT: The number of simultaneously active timeouts. */
#define MEMP_NUM_SYS_TIMEOUT (10 + LWIP_IPV6)

/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: The number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE 8

/* PBUF_POOL_BUFSIZE: The size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE 1524 + 16

/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/

#define CHECKSUM_BY_HARDWARE 0

/* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
#define CHECKSUM_GEN_IP 1

/* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
#define CHECKSUM_GEN_UDP 1

/* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
#define CHECKSUM_GEN_TCP 1

/* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
#define CHECKSUM_CHECK_IP 1

/* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
#define CHECKSUM_CHECK_UDP 1

/* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
#define CHECKSUM_CHECK_TCP 1

/* CHECKSUM_CHECK_ICMP==1: Check checksums by hardware for incoming ICMP packets.*/
#define CHECKSUM_GEN_ICMP 1

//#define LWIP_CHECKSUM_ON_COPY           1

/*
   ----------------------------------------------
   ---------- Sequential layer options ----------
   ----------------------------------------------
*/
/**
 * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN 0

/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/
/**
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET 1

#ifndef LWIP_DNS
#define LWIP_DNS 1
#endif
/*
   ------------------------------------
   ---------- httpd options ----------
   ------------------------------------
*/
/** Set this to 1 to include "fsdata_custom.c" instead of "fsdata.c" for the
 * file system (to prevent changing the file included in CVS) */
#define HTTPD_USE_CUSTOM_FSDATA 0

/*----- Default Value for LWIP_HTTPD_CGI: 0 ---*/
#define LWIP_HTTPD_CGI 1

/*----- Default Value for LWIP_HTTPD_SSI: 0 ---*/
#define LWIP_HTTPD_SSI 1

/*----- Default Value for LWIP_HTTPD_MAX_TAG_NAME_LEN: 8 ---*/
#define LWIP_HTTPD_MAX_TAG_NAME_LEN 16

/* LwIP Stack Parameters (modified compared to initialization value in opt.h) -*/
#define LWIP_HTTPD_DYNAMIC_HEADERS 1
/*
   ---------------------------------
   ---------- OS options ----------
   ---------------------------------
*/
#define TCPIP_THREAD_NAME         "TCP/IP"
#define TCPIP_THREAD_STACKSIZE    1000
#define TCPIP_MBOX_SIZE           6
#define DEFAULT_UDP_RECVMBOX_SIZE 6
#define DEFAULT_TCP_RECVMBOX_SIZE 6
#define DEFAULT_ACCEPTMBOX_SIZE   6
#define DEFAULT_THREAD_STACKSIZE  500

#define TCPIP_THREAD_PRIO 24 // osPriorityNormal

#ifndef LWIP_TESTMODE
#define LWIP_TESTMODE 0
#endif
/*
   ------------------------------------------------
   ---------- Network Interfaces options ----------
   ------------------------------------------------
*/

#define LWIP_NETIF_TX_SINGLE_PBUF 1

/*
   --------------------------------------
   ---------- Debugging options ---------
   --------------------------------------
*/
#ifdef LWIP_DEBUG

#define MEM_DEBUG        (LWIP_DBG_OFF)
#define MEMP_DEBUG       (LWIP_DBG_OFF)
#define PBUF_DEBUG       (LWIP_DBG_OFF)
#define API_LIB_DEBUG    (LWIP_DBG_OFF)
#define API_MSG_DEBUG    (LWIP_DBG_OFF)
#define TCPIP_DEBUG      (LWIP_DBG_OFF)
#define NETIF_DEBUG      (LWIP_DBG_OFF)
#define SOCKETS_DEBUG    (LWIP_DBG_OFF)
#define DEMO_DEBUG       (LWIP_DBG_OFF)
#define DHCP_DEBUG       (LWIP_DBG_OFF)
#define AUTOIP_DEBUG     (LWIP_DBG_OFF)
#define ETHARP_DEBUG     (LWIP_DBG_OFF)
#define IP_DEBUG         (LWIP_DBG_OFF)
#define IP_REASS_DEBUG   (LWIP_DBG_OFF)
#define IP6_DEBUG        (LWIP_DBG_OFF)
#define RAW_DEBUG        (LWIP_DBG_OFF)
#define ICMP_DEBUG       (LWIP_DBG_OFF)
#define ICMP6_DEBUG      (LWIP_DBG_OFF)
#define UDP_DEBUG        (LWIP_DBG_OFF)
#define TCP_DEBUG        (LWIP_DBG_OFF)
#define TCP_INPUT_DEBUG  (LWIP_DBG_OFF)
#define TCP_OUTPUT_DEBUG (LWIP_DBG_OFF)
#define TCP_RTO_DEBUG    (LWIP_DBG_OFF)
#define TCP_CWND_DEBUG   (LWIP_DBG_OFF)
#define TCP_WND_DEBUG    (LWIP_DBG_OFF)
#define TCP_FR_DEBUG     (LWIP_DBG_OFF)
#define TCP_QLEN_DEBUG   (LWIP_DBG_OFF)
#define TCP_RST_DEBUG    (LWIP_DBG_OFF)
#define PPP_DEBUG        (LWIP_DBG_OFF)
#endif /* LWIP_DEBUG */

#define LWIP_DBG_TYPES_ON \
  (LWIP_DBG_ON | LWIP_DBG_TRACE) /* (LWIP_DBG_ON|LWIP_DBG_TRACE|LWIP_DBG_STATE|LWIP_DBG_FRESH|LWIP_DBG_HALT) */

#endif /* __LWIPOPTS_H__ */
